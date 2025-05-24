#include <stdlib.h>
#include <log4c.h>
#include <unistd.h>
#include <string.h>


#include "client_handler.h"
#include "../users/users.h"
#include "../languages/languages.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../entities/utente.h"
#include "../../entities/lingua.h"
#include "../responses/responses.h"
#include "../validation/payload_validators.h"

int send_response(Client *client, const char *response);

int handle_register(const char *payload, char *error_msg, int *id_utente) {
    int res = SUCCESS;
    utente user = {0};
    log4c_category_t* log = initialize_log4c_category("handle_register");

    // Validate payload
    if((res = validate_register_user_payload(payload, error_msg, &user)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {// It's ok
        // Register user
        if ((res = registra_utente(&user)) != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to register user %s (ret code=%d).\n", user.email, res);
        } else {
            // Return id user
            *id_utente = user.id_utente;
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %s registered, id_user %ld.\n", user.email, user.id_utente);
        }
    }

    return res;
}

int handle_login(const char *payload, char *error_msg, utente *user) {
    int res = SUCCESS;
    int id_session = 0; // for future uses, I don't know if I'll need to manage sessions
    char email[128];
    char password[128];
    log4c_category_t* log = initialize_log4c_category("handle_login");

    if((res = validate_login_payload(payload, &id_session, email, password, error_msg)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {
        res = login(email, password, user);

        if (res == NO_DATA_FOUND) {
            strcpy(error_msg, "User not found");
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Login failed, %s\n", error_msg);
        }
        else if (res == WRONG_PASSWORD) {
            strcpy(error_msg, "Wrong password");
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Login failed, %s\n", error_msg);
        }
        else if (res != SUCCESS) {
            strcpy(error_msg, "Generic Error, failed to login user");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %s logged in\n", email);
            error_msg[0] = '\0';
        }
    }

    return res;
}

int handle_update_user(const char *payload, char *error_msg, char *ret_email) {
    log4c_category_t *log = initialize_log4c_category("handle_update_user");
    int res = 0;
    char email[128] = {0};
    char new_email[128] = {0};
    char new_password[128] = {0};
    char new_lang[10] = {0};
    char new_nome[128] = {0};
    char new_cognome[128] = {0};

    res = validate_update_user_payload(payload, error_msg, email, new_email, new_password, new_lang, new_nome, new_cognome);

    if(res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {

        // Proceed with update if password check was successful or not needed
        if (res == SUCCESS) {
            // Pass NULL for fields that are not being updated (empty strings from validation)
            const char *update_email = strlen(new_email) > 0 ? new_email : NULL;
            const char *update_password = strlen(new_password) > 0 ? new_password : NULL;
            const char *update_lang = strlen(new_lang) > 0 ? new_lang : NULL;
            const char *update_nome = strlen(new_nome) > 0 ? new_nome : NULL;
            const char *update_cognome = strlen(new_cognome) > 0 ? new_cognome : NULL;

            res = aggiorna_dati_utente(email, update_email, update_password, update_lang, update_nome, update_cognome);

            // If email was updated, subsequent operations should use the new email,
            // but the WHERE clause in upd_user uses the original email.
            // The log message reflects the outcome.
            if (res == SUCCESS) {
                 log4c_category_log(log, LOG4C_PRIORITY_INFO, "User data updated successfully for original email %s.\n", email);
                 strcpy(ret_email, update_email ? update_email : email);
            } else if (res == NO_DATA_FOUND) {
                 strcpy(error_msg, "User not found during update");
                 log4c_category_log(log, LOG4C_PRIORITY_WARN, "User %s not found during update attempt.\n", email);
            } else {
                 strcpy(error_msg, "Failed to update user data");
                 log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to update user data for %s (res=%d).\n", email, res);
            }
        }
        // If password check failed (res == WRONG_PASSWORD or NO_DATA_FOUND), res already holds the error code.
    }

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "handle_update_user finished for email %s with result: %d.\n", email, res);
    return res;
}

int handle_get_user(const char *payload, utente * user, char *error_msg) {

    log4c_category_t* log = initialize_log4c_category("handle_get_user");

    int res = SUCCESS;

    if((res = validate_get_user_payload(payload, error_msg, user->email, &(user->id_utente))) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {
        res = ottieni_utente(user);

        if (res != SUCCESS && res != NO_DATA_FOUND) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to get user\n");
        } else if (res == NO_DATA_FOUND) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User not found\n");
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User retrieved\n");

            log_user(user, log);
        }
    }

    return res;
}

int handle_get_languages(lingua ** langs, int *n_lingue, char *error_msg) {
    log4c_category_t* log = initialize_log4c_category("handle_get_languages");

    int res = SUCCESS;

    res = ottieni_lingue(langs, n_lingue);

    if (res != SUCCESS && res != NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to get languages\n");
    } else if (res == NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Languages not found\n");
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Languages retrieved\n");
    }

    return res;
}

int handle_get_language(const char *payload, lingua *lang, char *error_msg) {
    log4c_category_t* log = initialize_log4c_category("handle_get_language");

    int res = SUCCESS;

    if((res = validate_get_language_payload(payload, error_msg, lang) != SUCCESS)) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid payload (%d) - %s.\n", res, error_msg);
    }
    else {
        res = ottieni_lingua(lang->codice_lingua, lang->descrizione_lingua);

        if (res != SUCCESS && res != NO_DATA_FOUND) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to get language\n");
        } else if (res == NO_DATA_FOUND) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Language not found\n");
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Language retrieved (%s)\n", lang->descrizione_lingua);
        }
    }

    return res;
}

/// \brief Entry point of received messages, parses the headers and calls the correct handlers based on the payload
/// \param client_socket
/// \return
int handle_client_message(Client *client, char * buffer, int bytes_read) {
    char cmd[16];
    char payload[984];
    int res = SUCCESS;
    char error_msg[1024] = {0};

    log4c_category_t* log = initialize_log4c_category("handle_client_message");

    //buffer[bytes_read] = '\0'; // Null-terminate the received data

    if(buffer == NULL || bytes_read < 15 || strlen(buffer) != bytes_read) {
        strcpy(error_msg, "Invalid message received");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        char* resp = create_simple_response(STATUS_ERROR, error_msg);

        res = -2;
        if(send_response(client, resp) == SUCCESS)
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
    if (strcmp(cmd, "register       ") == 0) {
        int id_utente = 0;

        strcpy(payload, buffer + 15);
        if (strlen(payload) == 0) {
            strcpy(error_msg, "Missing payload");

            char* resp = create_simple_response(STATUS_ERROR, error_msg);

            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle register (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle register (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
            return -3;
        }

        res = handle_register(payload, error_msg, &id_utente);

        if(res != SUCCESS && res != EMAIL_ALREADY_USED) {
            char * resp = create_simple_response(STATUS_ERROR, error_msg);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle register (res=%d), response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle register (res=%d), response NOT sent.\n", res);

            free_response(&resp);
        }
        else if(res == EMAIL_ALREADY_USED) {
            char* resp = create_simple_response(STATUS_EMAIL_ALREADY_USED, "Email already used");
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "Email already used, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Email already used, response NOT sent.");

            free_response(&resp);
            res = SUCCESS;
        }
        else {
            char* resp = create_register_user_response(id_utente);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "User registered, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "User registered, response NOT sent.");
            free_response(&resp);
        }
    } else if (strncmp(cmd, "login          ", 15) == 0) {
        strcpy(payload, buffer + 15);
        if (strlen(payload) == 0) {
            strcpy(error_msg, "Missing payload");

            char *resp = create_simple_response(STATUS_ERROR, error_msg);

            if (send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle register (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle register (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
            return -5;
        }

        utente user;

        res = handle_login(payload, error_msg, &user);

        if (res != SUCCESS && res != WRONG_PASSWORD) {

            char *resp = create_simple_response(STATUS_ERROR, error_msg);
            if (send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle login (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle login (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
        } else if(res == WRONG_PASSWORD) {
            char *resp = create_simple_response(STATUS_LOGIN_WRONG_PASSWORD, error_msg);
            if (send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "User login, response sent (%s).", res == WRONG_PASSWORD ? STATUS_LOGIN_WRONG_PASSWORD : STATUS_OK);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "User login, response NOT sent (%s).", res == WRONG_PASSWORD ? STATUS_LOGIN_WRONG_PASSWORD : STATUS_OK);

            free_response(&resp);
            res = SUCCESS;
        }
        else {
            char *resp = create_get_user_response(&user);
            if (send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "User logged in (%s), response sent.", user.email);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "User logged in (%s), response NOT sent.", user.email);

            free_response(&resp);
        }
    } else if (strncmp(cmd, "get_language   ", 15) == 0) {
        strcpy(payload, buffer + 15);
        if (strlen(payload) == 0) {
            strcpy(error_msg, "Missing payload");

            char* resp = create_simple_response(STATUS_ERROR, error_msg);

            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle get_language (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle get_language (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
            return -7;
        }

        lingua lang = {0};

        res = handle_get_language(payload, &lang, error_msg);

        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "get_language res=%d.\n", res);

        if(res != SUCCESS && res != NO_DATA_FOUND) {
            char* resp = create_simple_response(STATUS_ERROR, "Failed to get language");
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle get_language (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle get_language (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
        }
        else if (res == NO_DATA_FOUND) {
            char * resp = create_simple_response(STATUS_NO_DATA_FOUND, "Language not found");
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "Language not found, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Language not found, response NOT sent.");

            free_response(&resp);
            res = SUCCESS;
        }
        else {
            char * resp = create_get_language_response(&lang);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "Language retrieved (%s), response sent.", lang.descrizione_lingua);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Language retrieved (%s), response NOT sent.", lang.descrizione_lingua);

            free_response(&resp);
        }
    } else if (strncmp(cmd, "get_languages  ", 15) == 0) {

        lingua *langs = NULL;
        int langs_count = 0;

        res = handle_get_languages(&langs, &langs_count, error_msg);

        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "handle_get_languages res=%d.\n", res);

        if(res != SUCCESS && res != NO_DATA_FOUND) {
            char* resp = create_simple_response(STATUS_ERROR, error_msg);

            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle get_languages (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle get_languages (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
        }
        else if (res == NO_DATA_FOUND) {
            char * resp = create_simple_response(STATUS_NO_DATA_FOUND, "No languages found");
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "No languages found, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "No languages found, response NOT sent.");

            free_response(&resp);
            res = SUCCESS;
        }
        else {
            char * resp = create_get_languages_response(langs, langs_count);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "Languages retrieved, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Languages retrieved, response NOT sent.");

            free_response(&resp);
        }

        if(langs != NULL) {
            free(langs);
            langs = NULL;
        }

    } else if (strncmp(cmd, "get_user       ", 15) == 0) {
        strcpy(payload, buffer + 15);
        if (strlen(payload) == 0) {
            strcpy(error_msg, "Missing payload");

            char* resp = create_simple_response(STATUS_ERROR, error_msg);

            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle get_user (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle get_user (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
            return -8;
        }

        utente user;
        res = handle_get_user(payload, &user, error_msg);

        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "handle_get_user res=%d.\n", res);

        if(res != SUCCESS && res != NO_DATA_FOUND) {
            char* resp = create_simple_response(STATUS_ERROR, error_msg);

            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle get_user (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle get_user (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
        }
        else if (res == NO_DATA_FOUND) {
            char * resp = create_simple_response(STATUS_NO_DATA_FOUND, "User not found");
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "User not found, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "User not found, response NOT sent.");

            free_response(&resp);
            res = SUCCESS;
        }
        else {
            char * resp = create_get_user_response(&user);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "User found, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "User found, response NOT sent.");

            free_response(&resp);
        }
    } else if (strncmp(cmd, "update_user    ", 15) == 0) {
        strcpy(payload, buffer + 15);
        if (strlen(payload) == 0) {
            strcpy(error_msg, "Missing payload");

            char* resp = create_simple_response(STATUS_ERROR, error_msg);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle update_user (res=%d). Response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle update_user (res=%d). Response NOT sent.\n", res);

            free_response(&resp);
            return -10;
        }

        char email[256] = {0};
        res = handle_update_user(payload, error_msg, email);

        if(res != SUCCESS && res != NO_DATA_FOUND && res != WRONG_PASSWORD) {
            char * resp = create_simple_response(STATUS_ERROR, error_msg);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle update_user (res=%d), response sent.\n", res);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle update_user (res=%d), response NOT sent.\n", res);

            free_response(&resp);
        }
        else if(res == NO_DATA_FOUND) {
            char *resp = create_simple_response(STATUS_NO_DATA_FOUND, "User not found");
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "User not found, response sent.");
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "User not found, response NOT sent.");

            free_response(&resp);
            res = SUCCESS;
        }
        else if(res == WRONG_PASSWORD) {
            char *resp = create_simple_response(STATUS_LOGIN_WRONG_PASSWORD, error_msg);
            if(send_response(client, resp) == SUCCESS)
                log4c_category_log(log, LOG4C_PRIORITY_INFO, "%s, response sent.", error_msg);
            else
                log4c_category_log(log, LOG4C_PRIORITY_WARN, "%s, response NOT sent.", error_msg);

            free_response(&resp);
            res = SUCCESS;
        }
        else {

            utente user;
            payload[0] = '\0';
            snprintf(payload, sizeof(payload), "{\"email\":\"%s\"}", email);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "handle_get_user payload: %s\n", payload);
            res = handle_get_user(payload, &user, error_msg);
            if (res != SUCCESS) {
                char * resp = create_simple_response(STATUS_ERROR, "User updated successully, but couldn't provide his info in the response");
                if(send_response(client, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to handle update_user (res=%d), response sent.\n", res);
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "Failed to handle update_user (res=%d), response NOT sent.\n", res);

                free_response(&resp);
            }
            else {
                char * resp = create_get_user_response(&user);
                
                if(send_response(client, resp) == SUCCESS)
                    log4c_category_log(log, LOG4C_PRIORITY_INFO, "User updated, response sent.");
                else
                    log4c_category_log(log, LOG4C_PRIORITY_WARN, "User updated, response NOT sent.");

                 free_response(&resp);
            }
        }

    } else if (strncmp(cmd, "close_auth_mgr ", 15) == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Received close_auth_mgr command\n");
        char * resp = create_simple_response(STATUS_OK, "Bye bye :)");
        if(send_response(client, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Response sent.");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Response NOT sent.");

        free_response(&resp);
        res = EXIT_COMMAND; // this will exit the application
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, still listening...\n");
        char * resp = create_simple_response(STATUS_ERROR, "Unsupported command, still listening...");
        if(send_response(client, resp) == SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, response sent.\n");
        else
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "Unsupported command, response NOT sent.\n");

        free_response(&resp);
    }

    return res;
}

int send_response(Client *client, const char *response) {
    log4c_category_t* log_sw = initialize_log4c_category("socket_write");
    log4c_category_log(log_sw, LOG4C_PRIORITY_DEBUG, "Sending response: %s\n", response);   
    if (client == NULL || response == NULL) {
        log4c_category_log(log_sw, LOG4C_PRIORITY_ERROR, "Client or response is NULL");
        return FAILURE;
    }
    if (client->socket < 0) {
        log4c_category_log(log_sw, LOG4C_PRIORITY_ERROR, "Invalid socket");
        return FAILURE;
    }
    // Lock the mutex to ensure thread safety
    // This is important to prevent concurrent writes to the socket from different threads

    if (!is_client_connected(client)) {
        log4c_category_log(log_sw, LOG4C_PRIORITY_WARN, "Client not connected for writing.");
        return FAILURE;
    }


    if (pthread_mutex_lock(&(client->write_mutex)) != 0) {
        perror("socket_write: pthread_mutex_lock");
        return FAILURE;
    }

    char response_with_newline[strlen(response) + 2];
    snprintf(response_with_newline, sizeof(response_with_newline), "%s\n", response);

    int bytes_written = write(client->socket, response_with_newline, strlen(response_with_newline));
    if (bytes_written < 0) {
        perror("send_response");
        pthread_mutex_unlock(&(client->write_mutex));
        return FAILURE;
    }

    pthread_mutex_unlock(&(client->write_mutex));
    return SUCCESS;

}