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

char* create_translate_response(const char *language_code_from, const char *language_code_to, const char *original_msg, const char *translated_msg) {
    log4c_category_t* log = initialize_log4c_category("create_translate_response");

    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();

    if (root == NULL || payload == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        cJSON_Delete(payload);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    cJSON_AddStringToObject(payload, "lang_from", language_code_from);
    cJSON_AddStringToObject(payload, "lang_to", language_code_to);
    cJSON_AddStringToObject(payload, "original_msg", original_msg);
    cJSON_AddStringToObject(payload, "translated_msg", translated_msg);

    cJSON_AddItemToObject(root, "payload", payload);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response created: %s", response_str);
    return response_str;
}

char* create_supported_languages_response(const TargetLanguage *languages, int count) {
    log4c_category_t* log = initialize_log4c_category("create_supported_languages_response");

    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateArray();

    if (root == NULL || payload == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        cJSON_Delete(payload);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    for (int i = 0; i < count; i++) {
        cJSON *lang_obj = cJSON_CreateObject();
        if (lang_obj == NULL) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object for language");
            continue;
        }
        cJSON_AddStringToObject(lang_obj, "language", languages[i].language);
        cJSON_AddStringToObject(lang_obj, "name", languages[i].name);
        cJSON_AddBoolToObject(lang_obj, "supports_formality", languages[i].supports_formality);
        cJSON_AddItemToArray(payload, lang_obj);
    }

    cJSON_AddItemToObject(root, "payload", payload);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return response_str;
}

void free_response(char** response) {
    if(response != NULL) {
        free(*response);
        *response = NULL;
    }
}