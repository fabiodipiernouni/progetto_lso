#include "socket_mgr.h"
#include "../utils/helper.h"
#include "handlers/client_handler.h"
#include "../utils/mapping.h"
#include "../repository/database.h"

#include <log4c.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <pthread.h>

typedef struct {
    Client *client;
    char* buffer;
    size_t buffer_size;
} ThreadData;

void* handle_client_message_thread(void* arg);

// Global private variables to manage the exit flag with mutex lock
static volatile int exit_flag = 0;
static pthread_mutex_t exit_flag_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to set the exit flag
void set_exit_flag(void) {
    pthread_mutex_lock(&exit_flag_mutex); // Lock the mutex
    exit_flag = 1;
    pthread_mutex_unlock(&exit_flag_mutex); // Unlock the mutex
}

// Function to check the exit flag
bool should_exit(void) {
    int result;
    pthread_mutex_lock(&exit_flag_mutex);
    result = exit_flag;
    pthread_mutex_unlock(&exit_flag_mutex);
    return result != 0;
}

bool is_client_connected(Client *client) {
    return client->socket > 0;
}

void on_client_disconnect(Client *client) {
    printf("Client %d disconnected - %s:%d\n", client->socket, inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));
    close(client->socket);
    client->socket = 0;
}

int create_server_socket(int *server_socket, struct sockaddr_in*server_addr, int port) {

    log4c_category_t* log = initialize_log4c_category("create_server_socket");

    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create socket\n");
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "setsockopt(SO_REUSEADDR) failed");
        close_server_socket(server_socket, NULL);
        return -2;
    }

    if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "setsockopt(SO_REUSEPORT) failed");
        close_server_socket(server_socket, NULL);
        return -3;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Socket created\n");

    if(server_addr == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "server_addr is NULL\n");
        close_server_socket(server_socket, NULL);
        return -4;
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(port);

    if (bind(*server_socket, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to bind socket\n");
        perror("bind");
        close(*server_socket);
        return -5;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Socket bound\n");

    if (listen(*server_socket, 5) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to listen\n");
        perror("listen");
        close(*server_socket);
        return -6;
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Server listening on port %d\n", port);

    return SUCCESS;
}
// Core function of the application, manages incoming and outgoing connections and database connections
int manage_connections(int server_socket, Client*clients) {
    log4c_category_t *log = initialize_log4c_category("manage_connections");

    if(server_socket == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Server socket is not initialized\n");
        return -1;
    }

    fd_set read_fds;
    int max_sd, activity;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (is_client_connected(clients + i)) {
                FD_SET(clients[i].socket, &read_fds);
                if (clients[i].socket > max_sd) {
                    max_sd = clients[i].socket;
                }
            }
        }

        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Select error\n");
            perror("Select error");
            return -2;
        }

        if(should_exit()) {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Exit flag set. Exiting manage_connections loop.\n");
            return EXIT_COMMAND;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            if (client_socket < 0) {
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to accept connection\n");
                perror("accept");
                return -3;
            }

            log4c_category_log(log, LOG4C_PRIORITY_INFO, "New connection, socket fd is %d, ip is : %s, port : %d\n", client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (!is_client_connected(clients + i)) {
                    clients[i].socket = client_socket;
                    clients[i].address = client_addr;
                    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Added client at index %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (is_client_connected(clients + i) && FD_ISSET(clients[i].socket, &read_fds)) {
                char buffer[1024];
                int bytes_read = read(clients[i].socket, buffer, sizeof(buffer));
                if (bytes_read == 0) {
                    on_client_disconnect(clients + i);
                } else {
                    buffer[bytes_read] = '\0';
                    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Received message from client %d: [%s]\n", clients[i].socket, buffer);

                    bool ok_thread = true;
                    pthread_t thread_id;
                    int thread_res = 0;
                    ThreadData *thread_data = NULL;

                    if(strncmp(buffer, "close_auth_mgr ", 15) == 0) {
                        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Received close_auth_mgr command. No thread will be created.\n");
                        ok_thread = false;
                    }

                    if(ok_thread) {
                        thread_data = malloc(sizeof(ThreadData));

                        if (thread_data == NULL) {
                            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate memory for thread data\n");
                            ok_thread = false;
                        }
                    }

                    if(ok_thread) {
                        thread_data->client = &clients[i];
                        thread_data->buffer = strdup(buffer);  // Copy the buffer to avoid race conditions
                        if (thread_data->buffer == NULL) {
                            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate memory for thread buffer\n");
                            free(thread_data);
                            ok_thread = false;
                        }
                    }


                    if(ok_thread) {
                        thread_data->buffer_size = bytes_read;

                        thread_res = pthread_create(&thread_id, NULL, handle_client_message_thread, thread_data);

                        if (thread_res != 0) {
                            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create thread for client %d. This will be worked by main thread.\n", clients[i].socket);
                            free(thread_data);
                            ok_thread = false;
                        }
                    }

                    if (!ok_thread) {
                        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create thread for client %d. This will be worked by main thread.\n", clients[i].socket);
                        int ret = handle_client_message(&clients[i], buffer, bytes_read);
                        if(ret == EXIT_COMMAND) // Close server
                        {
                            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Server will be closed\n");
                            return EXIT_COMMAND;
                        }
                        else if(ret != 0) {
                            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle message (ret code %d).\n", ret);
                        }
                        else {
                            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Message handled successfully.\n");
                        }
                    }
                    else
                        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Thread created with ID %lu for client %d\n", thread_id, clients[i].socket);
                }
            }
        }

        pthread_mutex_lock(&db_mutex);

        // Add DB connection check at the end of the loop
        static time_t last_check = 0;
        time_t now = time(NULL);
        if (now - last_check > 60) { // Check every minute
            if (check_db_connection() != SUCCESS) {
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Database connection check failed and reconnection attempt unsuccessful");
            }
            last_check = now;
        }

        pthread_mutex_unlock(&db_mutex);
    }
}

void* handle_client_message_thread(void* arg) {
    pthread_mutex_lock(&db_mutex);
    ThreadData* data = (ThreadData*)arg;

    // Call to the original function
    int ret = handle_client_message(data->client, data->buffer, data->buffer_size);

    // Manage the return value in case of EXIT_COMMAND
    if (ret == EXIT_COMMAND) {
        // Report to the main thread to close the server
        set_exit_flag();
    }

    // Free the allocated memory
    free(data->buffer);
    free(data);

    pthread_mutex_unlock(&db_mutex);

    return NULL;
}

int close_server_socket(int * server_socket, Client*clients) {
    log4c_category_t* log = initialize_log4c_category("close_server_socket");

    if(clients != NULL) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (is_client_connected(clients + i)) {
                close(clients[i].socket);
                clients[i].socket = 0;
                pthread_mutex_destroy(&(clients[i].write_mutex));
                log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Closed client socket %d\n", clients[i].socket);
            }
        }
    }

    close(*server_socket);
    *server_socket = 0;

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server socket closed\n");

    return 0;
}