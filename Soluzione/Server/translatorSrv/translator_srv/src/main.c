#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>

#include "utils/mapping.h"
#include "utils/helper.h"
#include "utils/rest_client.h"
#include "config/config_mgr.h"
#include <log4c.h>
#include "services/socket_mgr.h"
#include "services/handlers/client_handler.h"

volatile sig_atomic_t keep_running = 1;

void signal_handler(int signum);

int main() {
    // Configure signal handler
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
 
    int port = 0;
 
    port = get_port_from_config("resources/config/config.yaml");
    if (port <= 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid port number\n");
        return FAILURE;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Port number: %d\n", port);

    if(init_deepl_api_config("resources/config/config.yaml") != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to initialize Deepl API key\n");
        return FAILURE;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Deepl API key initialized: %s\n", get_deepl_api_config().api_key);

    struct sockaddr_in server_addr;
    int server_socket;
    if(create_server_socket(&server_socket, &server_addr, port) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create server socket\n");
        return FAILURE;
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Server listening on port %d", port);

    // Main server loop
    while (keep_running) {
        // Accept and handle new connection in separate thread
        if (accept_new_client(server_socket) < 0) {
            log4c_category_warn(log, "Failed to accept and handle client");
            // Short pause to avoid overloading CPU in case of repeated errors
            usleep(100000); // 100ms
        }
    }

    close_server_socket(&server_socket, NULL);

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Shutting down server..\n");
    log4c_fini(); // Clean up log4c

    return SUCCESS;
}

void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        keep_running = 0; // Set variable to 0 to terminate the loop
    }
}