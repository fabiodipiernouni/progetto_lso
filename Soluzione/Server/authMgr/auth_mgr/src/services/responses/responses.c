#include "responses.h"
#include "../../utils/helper.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <log4c.h>

char* create_simple_response(const char* esito, const char* message) {
    log4c_category_t* log = initialize_log4c_category("create_base_response");

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", esito);
    cJSON_AddStringToObject(root, "payload", message);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return response_str;
}

char* create_register_user_response(int id_utente) {
    log4c_category_t* log = initialize_log4c_category("register_user_response");

    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);
    cJSON_AddNumberToObject(root, "id_utente", id_utente);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response created: %s", response_str);
    return response_str;
}

char* create_get_user_response(const utente* user) {
    log4c_category_t* log = initialize_log4c_category("get_user_response");

    if (user == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Utente NULL");
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();

    if (root == NULL || payload == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    cJSON_AddNumberToObject(payload, "id_utente", user->id_utente);
    cJSON_AddStringToObject(payload, "nome", user->nome);
    cJSON_AddStringToObject(payload, "cognome", user->cognome);
    cJSON_AddStringToObject(payload, "email", user->email);
    cJSON_AddStringToObject(payload, "codice_lingua", user->codice_lingua);

    cJSON_AddItemToObject(root, "payload", payload);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response created: %s", response_str);
    return response_str;
}

char* create_get_language_response(const lingua* lang) {
    log4c_category_t* log = initialize_log4c_category("get_language_response");

    if(lang == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Lingua NULL");
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();

    if(root == NULL || payload == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        cJSON_Delete(payload);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    cJSON_AddStringToObject(payload, "codice_lingua", lang->codice_lingua);
    cJSON_AddStringToObject(payload, "descrizione", lang->descrizione_lingua);

    cJSON_AddItemToObject(root, "payload", payload);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response created: %s", response_str);

    return response_str;
}

char* create_get_languages_response(const lingua* langs, const int n_langs) {
    log4c_category_t* log = initialize_log4c_category("get_languages_response");

    if(langs == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Lingue NULL");
        return NULL;
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Building response for %d languages", n_langs);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateArray();

    if(root == NULL || payload == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    for(int i = 0; i < n_langs; i++) {
        cJSON *lang = cJSON_CreateObject();
        cJSON_AddStringToObject(lang, "codice_lingua", langs[i].codice_lingua);
        cJSON_AddStringToObject(lang, "descrizione", langs[i].descrizione_lingua);
        cJSON_AddItemToArray(payload, lang);
    }

    cJSON_AddItemToObject(root, "payload", payload);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response created: %s", response_str);

    return response_str;
}

void free_response(char** response) {
    if(response != NULL) {
        free(*response);
        *response = NULL;
    }
}