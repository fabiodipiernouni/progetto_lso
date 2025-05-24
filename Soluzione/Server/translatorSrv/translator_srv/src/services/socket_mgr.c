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

bool is_client_connected(Client *client) {
    return client->socket > 0;
}

int create_server_socket(int *server_socket, struct sockaddr_in *server_addr, int port) {

    log4c_category_t* log = initialize_log4c_category("create_server_socket");

    // Create a new TCP socket
    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create socket\n");
        perror("socket");
        return -1;
    }

    int opt = 1;
    // Set SO_REUSEADDR option to allow reusing local addresses quickly after server restart
    if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "setsockopt(SO_REUSEADDR) failed");
        close_server_socket(server_socket, NULL);
        return -2;
    }

    // Set SO_REUSEPORT option to allow multiple processes to bind to the same port
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

    // Configure server address structure:
    // - IPv4 address family
    // - Listen on all available interfaces (INADDR_ANY)
    // - Use the specified port number (converted to network byte order)
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(port);

    // Bind the socket to the configured address and port
    if (bind(*server_socket, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to bind socket\n");
        perror("bind");
        close(*server_socket);
        return -5;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Socket bound\n");

    // Start listening for incoming connections with a backlog queue of 5 pending connections
    if (listen(*server_socket, 5) < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to listen\n");
        perror("listen");
        close(*server_socket);
        return -6;
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Server listening on port %d\n", port);

    return SUCCESS;
}

int close_server_socket(int * server_socket, Client*clients) {
    log4c_category_t* log = initialize_log4c_category("close_server_socket");

    if(clients != NULL) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (is_client_connected(clients + i)) {
                close(clients[i].socket);
                clients[i].socket = 0;
            }
        }
    }

    close(*server_socket);
    *server_socket = 0;

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server socket closed\n");

    return 0;
}