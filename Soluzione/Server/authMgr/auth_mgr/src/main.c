#include "repository/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>

#include "utils/mapping.h"
#include "utils/helper.h"
#include "config/config_mgr.h"
#include <log4c.h>
#include "services/socket_mgr.h"

void signal_handler(int signum);

int main() {

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    if (log4c_init()) {
        fprintf(stderr, "log4c_init() failed\n");
        return FAILURE;
    }

    log4c_category_t* log = initialize_log4c_category("main");

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Initialize Database Connection...\n");

    int port = 0;

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Initializing database connection...\n");
    if (init_db("resources/config/config.yaml") != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to initialize database\n");
        return FAILURE;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Database initialized.\n");

    port = get_port_from_config("resources/config/config.yaml");

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Port number from config: %d\n", port);

    if (port <= 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid port number\n");
        return FAILURE;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Port number: %d\n", port);

    struct sockaddr_in server_addr;
    int server_socket;
    if(create_server_socket(&server_socket, &server_addr, port) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create server socket\n");
    }else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server socket created\n");

        Client clients[MAX_CLIENTS];
        for (int i = 0; i < MAX_CLIENTS; i++) {
            clients[i].socket = 0;
            if (pthread_mutex_init(&(clients[i].write_mutex), NULL) != 0) {
                perror("Failed to initialize client write mutex");
                exit(EXIT_FAILURE);
            }
        }

        int ret = manage_connections(server_socket, clients);

        if (ret == EXIT_COMMAND)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Server shutdown requested\n");
        else
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to manage connections\n");


        close_server_socket(&server_socket, clients);
    }

    close_db();
    terminate_db();
    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Shutdown server..\n");
    log4c_fini(); // Clean up log4c

    return SUCCESS;
}

void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        set_exit_flag();
    }
}