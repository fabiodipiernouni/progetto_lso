#include <stdio.h>
#include <stdlib.h>
#include <log4c.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdbool.h>
#include "payload_validators.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../entities/lingua.h"
#include "../../utils/rest_client.h"

bool isSupportedLanguage(const char *lang) {
    log4c_category_t *log = initialize_log4c_category("isSupportedLanguage");
    TargetLanguage *languages = NULL;
    bool is = false;

    int count = 0;

    char error_msg[1024] = {0};

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Checking if language is supported: %s\n", lang);

    int res = get_supported_languages(error_msg, &languages, &count);

    if (res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        if(languages != NULL) free(languages);
        return false;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Supported languages retrieved: %d\n", count);

    for (int i = 0; i < count; i++) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Checking language: %s\n", languages[i].language);
        if (strcmp(languages[i].language, lang) == 0) {
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Language %s is supported\n", lang);
            is = true;
            break;
        }
    }

    free(languages);
    return is;
}

int validate_translate_payload(const char *payload, char*error_msg, char *language_code_from, char *language_code_to, char *original_msg) {
    int res = SUCCESS;
    log4c_category_t *log = initialize_log4c_category("validate_translate_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_JSON;
    }

    // Check message
    if (res == SUCCESS) {
        cJSON *message_json = cJSON_GetObjectItemCaseSensitive(json, "message");

        if (message_json == NULL) {
            strcpy(error_msg, "Message field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (message_json->type != cJSON_String) {
            strcpy(error_msg, "Message field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(message_json->valuestring) == 0) {
            strcpy(error_msg, "Message field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else
            strcpy(original_msg, message_json->valuestring);
    }

    if (res == SUCCESS) {
        cJSON *lang_from_json = cJSON_GetObjectItemCaseSensitive(json, "lang_from");

        if (lang_from_json == NULL) {
            strcpy(error_msg, "lang_from field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (lang_from_json->type != cJSON_String) {
            strcpy(error_msg, "lang_from field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(lang_from_json->valuestring) == 0) {
            strcpy(error_msg, "lang_from field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else if(!isSupportedLanguage(lang_from_json->valuestring)) {
            strcpy(error_msg, "lang_from field not recognized");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = WRONG_LANGUAGE_CODE;
        } else
            strcpy(language_code_from, lang_from_json->valuestring);
    }

    if (res == SUCCESS) {
        cJSON *lang_to_json = cJSON_GetObjectItemCaseSensitive(json, "lang_to");

        if (lang_to_json == NULL) {
            strcpy(error_msg, "lang_to field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (lang_to_json->type != cJSON_String) {
            strcpy(error_msg, "lang_to field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(lang_to_json->valuestring) == 0) {
            strcpy(error_msg, "lang_to field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else if(!isSupportedLanguage(lang_to_json->valuestring)) {
            strcpy(error_msg, "lang_to field not recognized");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = WRONG_LANGUAGE_CODE;
        } else
            strcpy(language_code_to, lang_to_json->valuestring);
    }

    cJSON_Delete(json);
    return res;
}

