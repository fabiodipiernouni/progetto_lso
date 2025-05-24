#include <stdlib.h>
#include <log4c.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "client_handler.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../responses/responses.h"
#include "../requests/requests.h"
#include "../validation/payload_validators.h"
#include <signal.h>

#include "../users/users.h"
#include "../rooms/rooms.h"
#include "../auth/auth.h"
#include "../socket_mgr.h"

typedef struct {
    Client* client;
    AppContext* app_ctx;
} ThreadData;


void* thread_handle_client(void* args) {
    ThreadData* data = (ThreadData*)args;
    Client* client = data->client;
    AppContext* app_ctx = data->app_ctx;

    log4c_category_t* log = initialize_log4c_category("thread_handle_client");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Thread started for client socket %d\n", client->socket);

    char buffer[2032];
    int bytes_read;

    while ((bytes_read = read(client->socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the received data
        int result = handle_client_message(app_ctx, client, buffer, bytes_read);

        if (result == EXIT_COMMAND) {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Close command received, closing thread");
            break; // Exit the loop and close the thread
        }
    }

    on_client_disconnect(app_ctx, client);
    free(data); // Free the allocated memory for thread data
    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Client disconnected, closing thread");
    return NULL;
}

int accept_new_client(AppContext* app_ctx, int server_socket) {
    log4c_category_t* log = initialize_log4c_category("accept_new_client");
    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_socket =  accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Error accepting connection: %s", strerror(errno));
        return -1;
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "New connection accepted: socket %d", client_socket);

    int i = 0;
    for (; i < MAX_CLIENTS; i++) {
        if (!is_client_connected(app_ctx->clients + i)) {
            app_ctx->clients[i].socket = client_socket;
            app_ctx->clients[i].address = client_addr;
            app_ctx->clients[i].indice_client = i;
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Client assigned to slot %d", i);
            break;
        }
    }

    if (i == MAX_CLIENTS) {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "Max clients reached, connection rejected");
        close(client_socket);
        return -1;
    }

    pthread_t thread_id;
    ThreadData* thread_data = malloc(sizeof(ThreadData));
    thread_data->client = &app_ctx->clients[i];
    thread_data->app_ctx = app_ctx;

    if (pthread_create(&thread_id, NULL, thread_handle_client, thread_data) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Error creating thread: %s", strerror(errno));
        free(thread_data);
        close(client_socket);
        return -1;
    }

    pthread_detach(thread_id);
    return 0;
}

void notify_clients_of_room_change(AppContext* app_ctx, int indice_stanza) {
    
    log4c_category_t* log = initialize_log4c_category("notify_client_of_room_refresh");

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Notifying clients of room change\n");

    if(indice_stanza == -1)
        return;

    char req[1024];
    int res;

    if((res = create_refresh_room_request(indice_stanza, req)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create refresh request (res=%d)\n", res);
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Refresh request created successfully\n");

        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(is_client_connected(&app_ctx->clients[i])) {
                if(socket_write(app_ctx->clients[i].socket, req) != SUCCESS) {
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to send refresh request to client %d\n", app_ctx->clients[i].indice_client);
                }
            }
        }
    }
}

int handle_create_room(AppContext* app_ctx, const char *payload, char *error_msg, int indice_utente_host, int *indice_stanza) {
    int res = SUCCESS;

    char room_name[MAX_NOME_STANZA];
    ordine_gioco_t ordine_gioco;

    log4c_category_t* log = initialize_log4c_category("handle_create_room");

    //validate payload
    if((res = validate_create_room_payload(app_ctx, payload, error_msg, room_name, &ordine_gioco)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {//it's ok

        if ((res = create_room(app_ctx, indice_utente_host, room_name, ordine_gioco, indice_stanza)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create room %s (ret code=%d).\n", room_name, res);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Room %s created, id_room %d.\n", room_name, *indice_stanza);
            notify_clients_of_room_change(app_ctx, *indice_stanza);
        }

    }

    return res;
}

int handle_join_room(AppContext* app_ctx, const char *payload, char *error_msg, int id_utente, int *indice_stanza) {
    int res = SUCCESS;

    log4c_category_t* log = initialize_log4c_category("handle_join_room");

    //validate payload
    if((res = validate_join_room_payload(app_ctx, payload, error_msg, indice_stanza)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {//it's ok

        if ((res = join_room(app_ctx, id_utente, *indice_stanza)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to join room %d (ret code=%d).\n", *indice_stanza, res);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %d joined room %d.\n", id_utente, *indice_stanza);
            notify_clients_of_room_change(app_ctx, *indice_stanza);
        }

    }

    return res;
}

int handle_leave_room(AppContext* app_ctx, const char *payload, char *error_msg, int indice_utente, int *indice_stanza) {
    int res = SUCCESS;

    log4c_category_t* log = initialize_log4c_category("handle_leave_room");

    //validate payload
    if((res = validate_leave_room_payload(app_ctx, payload, error_msg, indice_stanza)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {//it's ok

        if ((res = leave_room(app_ctx, indice_utente, *indice_stanza)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to leave room %d (ret code=%d).\n", *indice_stanza, res);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %d left room %d.\n", indice_utente, *indice_stanza);
            notify_clients_of_room_change(app_ctx, *indice_stanza);
        }

    }

    return res;
}

int handle_get_room(AppContext* app_ctx, const char *payload, char *error_msg, int *indice_stanza) {
    int res = SUCCESS;

    log4c_category_t* log = initialize_log4c_category("handle_get_room");

    //validate payload
    if((res = validate_get_room_payload(app_ctx, payload, error_msg, indice_stanza)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {//it's ok
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Get room request received.\n");
    }

    return res;
}

int handle_start_game(AppContext* app_ctx, const char *payload, char *error_msg, int indice_utente, int *indice_stanza) {
    int res = SUCCESS;

    log4c_category_t* log = initialize_log4c_category("handle_start_game");

    //validate payload
    if((res = validate_start_game_payload(app_ctx, payload, error_msg, indice_stanza)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {//it's ok
        if ((res = start_game(app_ctx, indice_utente, *indice_stanza)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to start game in room %d (ret code=%d).\n", *indice_stanza, res);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Game started in room %d.\n", *indice_stanza);
            notify_clients_of_room_change(app_ctx, *indice_stanza);
        }
    }

    return res;
}

int handle_send_message(AppContext* app_ctx, const char *payload, char *error_msg, int indice_utente, int *indice_stanza) {
    int res = SUCCESS;

    log4c_category_t* log = initialize_log4c_category("handle_send_message");

    char message[MAX_LUNGHEZZA_MESSAGGIO];
    //validate payload
    if((res = validate_send_message_payload(payload, error_msg, message)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {//it's ok
        int current_indice_stanza = get_room_by_user_index(app_ctx, indice_utente);
        if(current_indice_stanza == -1) {
            strcpy(error_msg, "User not in a room");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = USER_NOT_IN_ROOM;
        }
        else {
            if((res = send_message(app_ctx, indice_utente, current_indice_stanza, message)) != SUCCESS) {
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to send message (ret code=%d).\n", res);
            } else {
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "Message sent successfully by user %d in room %d.\n", indice_utente, current_indice_stanza);
                *indice_stanza = current_indice_stanza; // pass back the room index
                notify_clients_of_room_change(app_ctx, current_indice_stanza);
            }
        }
    }

    return res;
}

int handle_set_user(AppContext* app_ctx, const char *payload, char *error_msg, int indice_client) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("handle_set_user");

    char email[MAX_EMAIL_UTENTE];
    if((res = validate_set_user_payload(payload, error_msg, email)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    } else {

        if ((res = get_user_by_email(app_ctx, email, indice_client)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to set user (ret code=%d).\n", res);
        } else {

            if((res = check_same_user_id_already_set(app_ctx, app_ctx->users[indice_client].id_utente, indice_client)) != SUCCESS) {
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User already set by another client\n");
                strcpy(error_msg, "User already set by another client");
                return res;
            }
    
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User set successfully.\n");
        }
    }
    return res;
}

int handle_client_disconnect(AppContext* app_ctx, int id_utente, char *error_msg) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("handle_client_disconnect");
    int indice_stanza = get_room_by_user_index(app_ctx, id_utente);

    if ((res = disconnect_user(app_ctx, id_utente)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to disconnect user %d (ret code=%d).\n", id_utente, res);
        if(res == USER_NOT_SET) {
            strcpy(error_msg, "User didn't set id");
        } else {
            strcpy(error_msg, "Failed to disconnect user");
        }
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %d disconnected.\n", id_utente);
        notify_clients_of_room_change(app_ctx, indice_stanza);
    }

    return res;
}

/// \brief Entry point of received messages, it parses the header and calls the correct handler based on the command
/// \param client
/// \return
int handle_client_message(AppContext* app_ctx, Client *client, char * buffer, int bytes_read) {
    char cmd[16];
    char payload[984];
    int res = SUCCESS;
    char* resp = NULL;
    char error_msg[1024] = {0};
    int indice_stanza = -1;

    log4c_category_t* log = initialize_log4c_category("handle_client_message");

    //buffer[bytes_read] = '\0'; // Null-terminate the received data

    if(buffer == NULL || bytes_read < 15 || strlen(buffer) != bytes_read) {
        strcpy(error_msg, "Invalid message received");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        char* resp = create_simple_response(STATUS_ERROR, error_msg);

        res = -2;
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid message received (res=%d). Response sent.\n", res);
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Invalid message received (res=%d). Response NOT sent.\n", res);

        free_response(&resp);
        return res;
    }

    strncpy(cmd, buffer, 15);
    cmd[15] = '\0';

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Received command: [%s].\n", cmd);

    // Process the request
    if (strcmp(cmd, "set_user       ") == 0) {
        if (has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User already set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_ALREADY_SET, error_msg);
            res = USER_ALREADY_SET; // Specific error code for this case
        } else {
            strcpy(payload, buffer + 15);
            if (strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for set_user");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD; // Specific error code
            } else {
                log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Payload: [%s] received from client %d\n", payload, client->indice_client);
              
                if( (res = handle_set_user(app_ctx, payload, error_msg, client->indice_client)) != SUCCESS) {
                    resp = create_simple_response(STATUS_ERROR, error_msg);
                } else {
                    resp = create_get_user_response(app_ctx, client->indice_client);
                }
                
                if(socket_write(client->socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");    
                }

                free_response(&resp);

        }
    } else if (strcmp(cmd, "disconnect     ") == 0) {
        if(has_user_set_id(app_ctx, client->indice_client)) {

            res = handle_client_disconnect(app_ctx, client->indice_client, error_msg);

            if(res != SUCCESS) {
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle disconnect (res=%d)\n", res);
                resp = create_simple_response(STATUS_ERROR, error_msg);
            } 
        }
    
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Disconnected successfully\n");

        resp = create_simple_response(STATUS_OK, "Disconnect successful");

        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");
            
        free_response(&resp);

    } else if (strcmp(cmd, "create_room    ") == 0) {
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            strcpy(payload, buffer + 15);
            if (strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for create_room");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD;
            } else {
                log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Payload: [%s] received from client %d\n", payload, client->indice_client);
                res = handle_create_room(app_ctx, payload, error_msg, client->indice_client, &indice_stanza);

                switch(res) {
                    case SUCCESS:
                        resp = create_create_room_response(app_ctx, indice_stanza);
                        notify_clients_of_room_change(app_ctx, indice_stanza);
                        break;
                    case ROOM_LIMIT_REACHED:
                        resp = create_simple_response(STATUS_ROOM_LIMIT_REACHED, error_msg);
                        break;
                    case ROOM_NAME_TAKEN:
                        resp = create_simple_response(STATUS_ROOM_NAME_TAKEN, error_msg);
                        break;
                    case USER_ALREADY_IN_ROOM:
                        resp = create_simple_response(STATUS_USER_ALREADY_IN_ROOM, error_msg);
                        break;
                    default:
                        resp = create_simple_response(STATUS_ERROR, error_msg);
                        break;
                }
            }
        }

        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");

        free_response(&resp);
    } else if(strncmp(cmd, "join_room      ", 15) == 0) {
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            strcpy(payload, buffer + 15);
            if (strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for join_room");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD;
            } else {
                log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Payload: [%s] received from client %d\n", payload, client->indice_client);
                res = handle_join_room(app_ctx, payload, error_msg, client->indice_client, &indice_stanza);

                switch(res) {
                    case SUCCESS:
                        resp = create_get_room_response(app_ctx, indice_stanza);
                        notify_clients_of_room_change(app_ctx, indice_stanza);
                        break;
                    case ROOM_NOT_FOUND:
                        resp = create_simple_response(STATUS_ROOM_NOT_FOUND, error_msg);
                        break;
                    case USER_ALREADY_IN_ROOM:
                        resp = create_simple_response(STATUS_USER_ALREADY_IN_ROOM, error_msg);
                        break;
                    case QUEUE_FULL:
                        resp = create_simple_response(STATUS_QUEUE_FULL, error_msg);
                        break;
                    default:
                        resp = create_simple_response(STATUS_ERROR, error_msg);
                        break;
                }
            }
        }

        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");

        free_response(&resp);
    } else if(strncmp(cmd, "leave_room     ", 15) == 0) {
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            strcpy(payload, buffer + 15);
            if (strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for leave_room");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD;
            } else {
                log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Payload: [%s] received from client %d\n", payload, client->indice_client);
                res = handle_leave_room(app_ctx, payload, error_msg, client->indice_client, &indice_stanza);

                switch(res) {
                    case SUCCESS:
                        resp = create_simple_response(STATUS_OK, "Left room successfully");
                        notify_clients_of_room_change(app_ctx, indice_stanza);
                        break;
                    case ROOM_NOT_FOUND:
                        resp = create_simple_response(STATUS_ROOM_NOT_FOUND, error_msg);
                        break;
                    case USER_NOT_IN_ROOM:
                        resp = create_simple_response(STATUS_USER_NOT_IN_ROOM, error_msg);
                        break;
                    default:
                        resp = create_simple_response(STATUS_ERROR, error_msg);
                        break;
                }
            }
        }

        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");

        free_response(&resp);
    } else if (strncmp(cmd, "start_game     ", 15) == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Received start_game command\n");
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            strcpy(payload, buffer + 15);
            if( strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for start_game");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD;
            } else {
                res = handle_start_game(app_ctx, payload, error_msg, client->indice_client, &indice_stanza);
                switch (res) {
                    case SUCCESS:
                        resp = create_get_room_response(app_ctx, indice_stanza);
                        notify_clients_of_room_change(app_ctx, indice_stanza);
                        break;
                    case ROOM_NOT_FOUND:
                        resp = create_simple_response(STATUS_ROOM_NOT_FOUND, error_msg);
                        break;
                    case USER_NOT_HOST:
                        resp = create_simple_response(STATUS_USER_NOT_HOST, error_msg);
                        break;
                    case GAME_ALREADY_STARTED:
                        resp = create_simple_response(STATUS_GAME_ALREADY_STARTED, error_msg);
                        break;
                    case NOT_ENOUGH_PLAYERS:
                        resp = create_simple_response(STATUS_NOT_ENOUGH_PLAYERS, error_msg);
                        break;
                    default:
                        resp = create_simple_response(STATUS_ERROR, error_msg);
                        break;
                }
            }
        }
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");
        free_response(&resp);
    } else if (strncmp(cmd, "send_message   ", 15) == 0) {
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            strcpy(payload, buffer + 15);
            if (strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for send_message");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD;
            } else {
                res = handle_send_message(app_ctx, payload, error_msg, client->indice_client, &indice_stanza);
                switch (res) {
                    case SUCCESS:
                        resp = create_get_room_response(app_ctx, indice_stanza);
                        // Notification is handled within handle_send_message
                        break;
                    case USER_NOT_IN_ROOM:
                        resp = create_simple_response(STATUS_USER_NOT_IN_ROOM, error_msg);
                        break;
                    case GAME_NOT_STARTED:
                        resp = create_simple_response(STATUS_GAME_NOT_STARTED, error_msg);
                        break;
                    case NOT_PLAYER_TURN:
                        resp = create_simple_response(STATUS_NOT_PLAYER_TURN, error_msg);
                        break;
                    case USER_ALREADY_SENT_MESSAGE:
                        resp = create_simple_response(STATUS_USER_ALREADY_SENT_MESSAGE, error_msg);
                        break;
                    default:
                        resp = create_simple_response(STATUS_ERROR, error_msg);
                        break;
                }
            }
        }
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");
        free_response(&resp);
    } else if (strncmp(cmd, "get_rooms      ", 15) == 0) {
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            resp = create_get_rooms_response(app_ctx);
            res = SUCCESS;
        }
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");
        free_response(&resp);
    } else if (strncmp(cmd, "get_room       ", 15) == 0) {
        if(!has_user_set_id(app_ctx, client->indice_client)) {
            strcpy(error_msg, "User not set");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            resp = create_simple_response(STATUS_USER_NOT_SET, error_msg);
            res = USER_NOT_SET;
        } else {
            strcpy(payload, buffer + 15);
            if (strlen(payload) == 0) {
                strcpy(error_msg, "Payload is empty for get_room");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                resp = create_simple_response(STATUS_ERROR, error_msg);
                res = EMPTY_FIELD;
            } else {
                res = handle_get_room(app_ctx, payload, error_msg, &indice_stanza);
                if (res == SUCCESS) {
                    resp = create_get_room_response(app_ctx, indice_stanza);
                } else {
                    resp = create_simple_response(STATUS_ROOM_NOT_FOUND, error_msg);
                }
            }
        }
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");
        free_response(&resp);
    } else if (strncmp(cmd, "close_game_rule", 15) == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Received close_game_rule command\n");
        resp = create_simple_response(STATUS_OK, "Bye bye :)");
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");

        free_response(&resp);
        res = EXIT_COMMAND; // this will exit the application
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, still listening...\n");
        resp = create_simple_response(STATUS_ERROR, "Unsupported command, still listening...");
        if(socket_write(client->socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, response sent.\n");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, response NOT sent.\n");

        free_response(&resp);
    }

    return res;
}