#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>


#include "utils/mapping.h"
#include "utils/helper.h"
#include "config/config_mgr.h"
#include <log4c.h>
#include "services/socket_mgr.h"
#include "utils/app_context.h"

#include "services/rooms/rooms.h"
#include "services/users/users.h"
#include "services/auth/auth.h"
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
    
    log4c_category_t* log = initialize_log4c_category("main");

    AppContext *app_ctx = initialize_app_context();
    if (!app_ctx) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to initialize app context\n");
        return FAILURE;
    }

    if (log4c_init()) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "log4c_init() failed\n");
        return FAILURE;
    }


    int port = 0;
    if(get_config_value("resources/config/config.yaml", "socket", NULL, &port) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to get socket configuration\n");
        return FAILURE;
    }

    if (port <= 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid port number\n");
        return FAILURE;
    }

    int server_socket;
    if(create_server_socket(app_ctx, &server_socket, port) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create server socket\\n");
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server socket created\\n");


        // Main server loop
        while (keep_running) {
            // Accept and handle new connection in separate thread
            if (accept_new_client(app_ctx, server_socket) < 0) {
                log4c_category_warn(log, "Failed to accept and handle client");
                // Short pause to avoid overloading CPU in case of repeated errors
                usleep(100000); // 100ms
            }
        }
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Shutdown server..\n");

    close_server_socket(app_ctx, &server_socket);
    destroy_app_context(app_ctx);
    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Server shutdown complete\n");

    log4c_fini(); // Clean up log4c

    return SUCCESS;
}

void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        keep_running = 0; // Set variable to 0 to terminate the loop
    }
}