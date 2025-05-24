#include <cjson/cJSON.h>
#include <string.h>
#include <log4c.h>
#include <unistd.h> // close()

#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../config/config_mgr.h"

#include "../socket_mgr.h"
#include "../responses/responses.h"
#include "../rooms/rooms.h"
#include "../users/users.h"

#include "auth.h"

int process_get_user_response(AppContext* app_ctx, char response[], int indice_utente) {
    log4c_category_t* log = initialize_log4c_category("process_get_user_response");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Processing get user response\n");

    cJSON *root = cJSON_Parse(response);
    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to parse JSON response\n");
        return FAILURE;
    }

    cJSON *esito = cJSON_GetObjectItem(root, "esito");
    if (esito == NULL || strcmp(esito->valuestring, STATUS_OK) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Get user failed: %s\n", esito->valuestring);
        cJSON_Delete(root);
        return FAILURE;
    }

    cJSON *payload = cJSON_GetObjectItem(root, "payload");
    if (payload == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Payload not found in response\n");
        cJSON_Delete(root);
        return FAILURE;
    }

    cJSON *id_utente_json = cJSON_GetObjectItem(payload, "id_utente");
    if (id_utente_json == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User ID not found in payload\n");
        cJSON_Delete(root);
        return FAILURE;
    }
    app_ctx->users[indice_utente].id_utente = id_utente_json->valueint;
    cJSON *email = cJSON_GetObjectItem(payload, "email");
    if (email != NULL) {
        strncpy(app_ctx->users[indice_utente].email, email->valuestring, sizeof(app_ctx->users[indice_utente].email) - 1);
        app_ctx->users[indice_utente].email[sizeof(app_ctx->users[indice_utente].email) - 1] = '\0'; // Ensure null termination
    }
    cJSON *nome = cJSON_GetObjectItem(payload, "nome");
    if (nome != NULL) {
        strncpy(app_ctx->users[indice_utente].nome, nome->valuestring, sizeof(app_ctx->users[indice_utente].nome) - 1);
        app_ctx->users[indice_utente].nome[sizeof(app_ctx->users[indice_utente].nome) - 1] = '\0'; // Ensure null termination
    }
    cJSON *cognome = cJSON_GetObjectItem(payload, "cognome");
    if (cognome != NULL) {
        strncpy(app_ctx->users[indice_utente].cognome, cognome->valuestring, sizeof(app_ctx->users[indice_utente].cognome) - 1);
        app_ctx->users[indice_utente].cognome[sizeof(app_ctx->users[indice_utente].cognome) - 1] = '\0'; // Ensure null termination
    }
    cJSON *codice_lingua = cJSON_GetObjectItem(payload, "codice_lingua");
    if (codice_lingua != NULL) {
        strncpy(app_ctx->users[indice_utente].codice_lingua, codice_lingua->valuestring, sizeof(app_ctx->users[indice_utente].codice_lingua) - 1);
        app_ctx->users[indice_utente].codice_lingua[sizeof(app_ctx->users[indice_utente].codice_lingua) - 1] = '\0'; // Ensure null termination
    }

    cJSON_Delete(root);
    return SUCCESS;
}

int get_user_by_email(AppContext* app_ctx, char email[], const int indice_client){
    log4c_category_t* log = initialize_log4c_category("get_user_by_email");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Getting user by email\n");

    int auth_mgr_socket_local = -1;
    char auth_ip[16] = {0};
    int auth_port = 0;

    if(get_config_value("resources/config/config.yaml", "auth_mgr", auth_ip, &auth_port) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to get auth_mgr configuration\n");
        return FAILURE;
    }

    if (auth_port <= 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid auth_mgr port number\n");
        return FAILURE;
    }

    if(create_client_socket(&auth_mgr_socket_local, auth_ip, auth_port) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create client socket to auth_mgr\n");
        return FAILURE;
    }
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Client socket to auth_mgr created successfully.\n");

    char payload[MAX_EMAIL_UTENTE+29] = {0};
    snprintf(payload, sizeof(payload), "get_user       {\"email\": \"%s\"}", email);
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "%s\n", payload);

    if (socket_write(auth_mgr_socket_local, payload) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to send request to auth_mgr\n");
        close(auth_mgr_socket_local);
        return FAILURE;
    }

    char response[1024] = {0};
    if (socket_read(auth_mgr_socket_local, response, sizeof(response)) != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to read response from auth_mgr\n");
        close(auth_mgr_socket_local);
        return FAILURE;
    }

    close(auth_mgr_socket_local);
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Client socket to auth_mgr closed.\n");

    return process_get_user_response(app_ctx, response, indice_client);
}