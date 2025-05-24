#include "../entities/utente.h"
#include "../entities/stanza.h"

#include "socket_mgr.h"
#include "../utils/helper.h"
#include "handlers/client_handler.h"
#include "../utils/mapping.h"

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
#include <netdb.h>

#include "../utils/app_context.h"

#include "handlers/client_handler.h"

bool is_client_connected(Client *client) {
    return client->socket > 0;
}

void on_client_disconnect(AppContext* app_ctx, Client *client_ptr) {
    log4c_category_t* log = initialize_log4c_category("on_client_disconnect");
    
    int client_idx = client_ptr->indice_client;

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Client %d (socket %d) disconnected - %s:%d\n", client_idx, client_ptr->socket, inet_ntoa(client_ptr->address.sin_addr), ntohs(client_ptr->address.sin_port));

    char error_msg[1024];
    int res;
    if((res = handle_client_disconnect(app_ctx, client_idx, error_msg)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle user disconnect for client_idx %d (res=%d)\n", client_idx, res);
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "User disconnect handled successfully for client_idx %d\n", client_idx);
    }

    close(client_ptr->socket);

    if (client_idx >= 0 && client_idx < MAX_CLIENTS) {
        app_ctx->clients[client_idx].socket = 0;
    }
}

int create_client_socket(int *client_socket, char ip[], int port) {

    log4c_category_t* log = initialize_log4c_category("create_client_socket");

    *client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*client_socket < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create socket\n");
        perror("socket");
        return -1;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Socket created\n");

    struct sockaddr_in server_addr;
    struct addrinfo hints, *results, *p;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;     // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(ip, NULL, &hints, &results)) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "getaddrinfo error: %s\n", gai_strerror(status));
        close(*client_socket);
        return -1;
    }

    // Use the first address returned by getaddrinfo
    p = results;
    memcpy(&server_addr, p->ai_addr, sizeof(struct sockaddr_in));
    server_addr.sin_port = htons(port);

    freeaddrinfo(results); // Free memory allocated by getaddrinfo

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Connecting to %s:%d\n", ip, port);

    if (connect(*client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to connect to server\n");
        perror("connect");
        close(*client_socket);
        return -2;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Socket connected\n");
    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Client connected to server at %s:%d\n", ip, port);

    return SUCCESS;
}

int create_server_socket(AppContext* app_ctx, int *server_socket, int port) {

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
        close_server_socket(app_ctx, server_socket);
        return -2;
    }

    if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "setsockopt(SO_REUSEPORT) failed");
        close_server_socket(app_ctx, server_socket);
        return -3;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Socket created\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    server_addr.sin_port = htons(port);

    if (bind(*server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
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

int close_server_socket(AppContext* app_ctx, int * server_socket) {
    log4c_category_t* log = initialize_log4c_category("close_server_socket");

    if(app_ctx != NULL && app_ctx->clients != NULL) { // Check app_ctx and app_ctx->clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (is_client_connected(&app_ctx->clients[i])) { // Pass address of client
                close(app_ctx->clients[i].socket);
                app_ctx->clients[i].socket = 0;
            }
        }
    }

    close(*server_socket);
    *server_socket = 0;

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server socket closed\n");

    return 0;
}


int socket_write(const int socket, const char *message) {
    
    if (socket < 0) {
        return FAILURE;
    }

    char message_with_newline[strlen(message) + 2];
    snprintf(message_with_newline, sizeof(message_with_newline), "%s\n", message);

    int bytes_written = write(socket, message_with_newline, strlen(message_with_newline));
    if (bytes_written < 0) {
        perror("socket_write: write");
        return FAILURE;
    }
    return SUCCESS;
}

int socket_read(const int socket, char response[], const int size) {
    int bytes_read = read(socket, response, size - 1);
    if (bytes_read < 0) {
        perror("socket_read");
        return FAILURE;
    }
    response[bytes_read] = '\0'; // Null-terminate the string
    return SUCCESS;
}