#include <stdlib.h>
#include <log4c.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>

#include "client_handler.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../responses/responses.h"
#include "../validation/payload_validators.h"
#include "../translator/translator.h"

void* thread_handle_client(void* args) {
    int client_socket = *(int*)args;
    free(args);

    log4c_category_t* log = initialize_log4c_category("thread_handle_client");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Thread started for client socket %d\n", client_socket);

    char buffer[2048];
    int bytes_read;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the received data
        int result = handle_client_message(client_socket, buffer, bytes_read);

        if (result == EXIT_COMMAND) {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Exit command received");
            // Propagate exit signal
            kill(getpid(), SIGTERM);
            break;
        }
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Client disconnected, closing thread");
    close(client_socket);
    return NULL;
}

int accept_new_client(int server_socket) {
    log4c_category_t* log = initialize_log4c_category("accept_new_client");

    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket < 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Error accepting connection: %s", strerror(errno));
        return -1;
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "New connection accepted: socket %d", client_socket);

    int* client_socket_ptr = malloc(sizeof(int));
    if (client_socket_ptr == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Error allocating memory");
        close(client_socket);
        return -1;
    }

    *client_socket_ptr = client_socket;

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, thread_handle_client, client_socket_ptr) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Error creating thread: %s", strerror(errno));
        free(client_socket_ptr);
        close(client_socket);
        return -1;
    }

    pthread_detach(thread_id);
    return 0;
}

int send_response(int client_socket, const char *response);

int handle_translate(const char *payload, char *error_msg, char *language_code_from, char *language_code_to, char *original_msg, char *translated_msg) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("handle_translate");

    // Validate payload
    if((res = validate_translate_payload(payload, error_msg, language_code_from, language_code_to, original_msg)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {// It's ok
        if ((res = translate(original_msg, error_msg, language_code_from, language_code_to, translated_msg)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to translate msg (ret code=%d).\n", res);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Message translated.\n");
        }
    }

    return res;
}



/// \brief Entry point dei messaggi ricevuti, ne fa parsing degli header e sulla base chiama gli handler corretti passando il payload
/// \param client_socket
/// \return
int handle_client_message(int client_socket, char * buffer, int bytes_read) {
    char cmd[16];
    char payload[2032] = {0};
    int res = SUCCESS;
    char error_msg[1024] = {0};

    log4c_category_t* log = initialize_log4c_category("handle_client_message");

    //buffer[bytes_read] = '\0'; // Null-terminate the received data

    if(buffer == NULL || bytes_read < 15 || strlen(buffer) != bytes_read) {
        strcpy(error_msg, "Invalid message received");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        char* resp = create_simple_response(STATUS_ERROR, error_msg);

        res = -2;
        if(send_response(client_socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid message received (res=%d). Response sent.\n", res);
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Invalid message received (res=%d). Response NOT sent.\n", res);

        free_response(&resp);
        return res;
    }

    strncpy(cmd, buffer, 15);
    cmd[15] = '\0';

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Received command: [%s].\n", cmd);

    // Process the request (this is a simplified example)
    if (strcmp(cmd, "translate      ") == 0) {

        char original_msg[MAX_MESSAGE_SIZE] = {0};
        char translated_msg[MAX_MESSAGE_SIZE] = {0};
        char language_code_from[10] = {0}, language_code_to[10] = {0};

        strcpy(payload, buffer + 15);
        if (strlen(payload) == 0) {
            strcpy(error_msg, "Missing payload");

            char* resp = create_simple_response(STATUS_ERROR, error_msg);

            if(send_response(client_socket, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle translate (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle translate (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
            return -3;
        }

        res = handle_translate(payload, error_msg, language_code_from, language_code_to, original_msg, translated_msg);

        char* resp = NULL;

        switch (res) {
            case INVALID_FIELD_TYPE:
                resp = create_simple_response(STATUS_INVALID_FIELD_TYPE, error_msg);
                if(send_response(client_socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle translate (res=%d), response sent.\n", res);
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle translate (res=%d), response NOT sent.\n", res);

                free_response(&resp);
                break;
            case EMPTY_FIELD:
                resp = create_simple_response(STATUS_EMPTY_FIELD, error_msg);
                if(send_response(client_socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle translate (res=%d), response sent.\n", res);
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle translate (res=%d), response NOT sent.\n", res);

                free_response(&resp);
                break;
            case MISSING_FIELD:
                resp = create_simple_response(STATUS_MISSING_FIELD, error_msg);
                if(send_response(client_socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle translate (res=%d), response sent.\n", res);
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle translate (res=%d), response NOT sent.\n", res);

                free_response(&resp);
                break;
            case WRONG_LANGUAGE_CODE:
                resp = create_simple_response(STATUS_WRONG_LANGUAGE_CODE, "Language code not recognized.");
                if(send_response(client_socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Wrong language code, response sent.");
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Wrong language code, response NOT sent.");

                free_response(&resp);
                break;
            case SUCCESS:
                resp = create_translate_response(language_code_from, language_code_to, original_msg, translated_msg);
                if(send_response(client_socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Translation successful, response sent.");
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Translation successful, response NOT sent.");
                free_response(&resp);
                break;
            default:
                resp = create_simple_response(STATUS_ERROR, error_msg);
                if(send_response(client_socket, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle translate (res=%d), response sent.\n", res);
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle translate (res=%d), response NOT sent.\n", res);

                free_response(&resp);
                break;
        }
    } else if (strncmp(cmd, "get_languages  ", 15) == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Received get_languages command\n");
        TargetLanguage *languages = NULL;
        int count = 0;

        res = get_supported_languages(error_msg, &languages, &count);

        char * resp = NULL;

        if(res == SUCCESS) {
            resp = create_supported_languages_response(languages, count);
            if(send_response(client_socket, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response get_languages sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response get_languages NOT sent.");

            free(languages);
            free_response(&resp);
        } else {
            resp = create_simple_response(STATUS_ERROR, error_msg);
            if(send_response(client_socket, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle get_languages (res=%d), response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle get_languages (res=%d), response NOT sent.\n", res);

            free_response(&resp);
        }
    } else if (strncmp(cmd, "close_trnsl_src", 15) == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Received close_trnsl_src command\n");
        char * resp = create_simple_response(STATUS_OK, "Bye bye :)");
        if(send_response(client_socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");

        free_response(&resp);
        res = EXIT_COMMAND; // this will exit the application
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, still listening...\n");
        char * resp = create_simple_response(STATUS_ERROR, "Unsupported command, still listening...");
        if(send_response(client_socket, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, response sent.\n");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, response NOT sent.\n");

        free_response(&resp);
    }

    return res;
}

int send_response(int client_socket, const char *response) {
    char response_with_newline[strlen(response) + 2];
    snprintf(response_with_newline, sizeof(response_with_newline), "%s\n", response);

    int bytes_written = write(client_socket, response_with_newline, strlen(response_with_newline));
    if (bytes_written < 0) {
        perror("send_response");
        return FAILURE;
    }
    return SUCCESS;

}