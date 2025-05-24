#include <string.h>
#include <curl/curl.h>
#include <log4c.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <unistd.h>

#include "../config/config_mgr.h"
#include "helper.h"
#include "mapping.h"

static DeeplAPI deeplAPIConfig;
static bool initialized = false;

int init_deepl_api_config(const char *config_file) {
    memset(&deeplAPIConfig, 0, sizeof(DeeplAPI));
    int res = get_deepl_api_from_config(config_file, &deeplAPIConfig);
    if(res == SUCCESS) initialized = true;

    return res;
}

const DeeplAPI get_deepl_api_config() {
    return deeplAPIConfig;
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    char **response_ptr = (char **)userp;
    size_t realsize = size * nmemb;

    // If it is the first block of data, allocate memory 
    if (*response_ptr == NULL) {
        *response_ptr = malloc(realsize + 1);
        if (*response_ptr == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 0;
        }
        memcpy(*response_ptr, contents, realsize);
        (*response_ptr)[realsize] = '\0';
    } else {
        // If we already have data, reallocate and concatenate
        size_t current_size = strlen(*response_ptr);
        char *new_mem = realloc(*response_ptr, current_size + realsize + 1);

        if (new_mem == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            return 0;
        }

        *response_ptr = new_mem;
        memcpy(*response_ptr + current_size, contents, realsize);
        (*response_ptr)[current_size + realsize] = '\0';
    }

    return realsize;
}

static char supportedLanguagesCache[8192] = {0};
static time_t cacheTimestamp = 0;
static const int CACHE_VALIDITY_HOURS = 5;

int get_supported_languages(char* error_msg, TargetLanguage **languages, int *count) {
    log4c_category_t *log = initialize_log4c_category("get_supported_languages");

    CURL *curl = NULL;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char url[384];
    char *response = NULL;

    if(!initialized) {
        strcpy(error_msg, "Deepl API not initialized");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        return FAILURE;
    }

    time_t currentTime = time(NULL);
    bool cacheValid = (strlen(supportedLanguagesCache) > 0) &&
        (cacheTimestamp != 0) && (difftime(currentTime, cacheTimestamp) < CACHE_VALIDITY_HOURS * 3600);

    if(!cacheValid) {
        memset(supportedLanguagesCache, 0, sizeof(supportedLanguagesCache));
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Cache expired and set to empty or cache empty.\n");
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Cache valid, will be alive for %d minutes.\n",
            (int)((CACHE_VALIDITY_HOURS * 3600 - difftime(currentTime, cacheTimestamp)) / 60));
    }

    if(strlen(supportedLanguagesCache) == 0) {

        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "No cache. Calling API.\n");

        snprintf(url, sizeof(url), "%s/languages?type=target", deeplAPIConfig.base_url);

        curl = curl_easy_init();
        if (!curl) {
            strcpy(error_msg, "Failed to initialize CURL");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            return FAILURE;
        }

        // Headers preparations
        headers = curl_slist_append(headers, "Content-Type: application/json");
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: DeepL-Auth-Key %s", deeplAPIConfig.api_key);
        headers = curl_slist_append(headers, auth_header);

        // cURL configuration
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            sprintf(error_msg, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            if (response != NULL) {
                free(response);
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return FAILURE;
        }

        if (response == NULL) {
            strcpy(error_msg, "No response received from server");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return FAILURE;
        }

        strcpy(supportedLanguagesCache, response);
        cacheTimestamp = time(NULL);
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Cache hit. Using cached response.\n");
        response = (char*)malloc(strlen(supportedLanguagesCache) + 1);
        if (response == NULL) {
            strcpy(error_msg, "Memory allocation failed for cached response");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            return FAILURE;
        }
        memset(response, 0, strlen(supportedLanguagesCache) + 1);
        strcpy(response, supportedLanguagesCache);
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response: %s\n", response);

    cJSON *json = cJSON_Parse(response);
    if(json == NULL) {
        strcpy(error_msg, "Error parsing JSON response");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        free(response);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return INVALID_JSON;
    }

    if(!cJSON_IsArray(json)) {
        strcpy(error_msg, "Response format is not an array");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        cJSON_Delete(json);
        free(response);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return INVALID_FORMAT;
    }

    *count = cJSON_GetArraySize(json);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Number of languages: %d\n", *count);

    *languages = malloc(sizeof(TargetLanguage) * (*count));

    if(*languages == NULL) {
        strcpy(error_msg, "Memory allocation failed for languages");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        cJSON_Delete(json);
        free(response);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return FAILURE;
    }

    memset(*languages, 0, sizeof(TargetLanguage) * (*count));

    for(int i = 0; i < (*count); i++) {
        cJSON *lang_item = cJSON_GetArrayItem(json, i);
        cJSON *language = cJSON_GetObjectItem(lang_item, "language");
        cJSON *name = cJSON_GetObjectItem(lang_item, "name");
        cJSON *supports_formality = cJSON_GetObjectItem(lang_item, "supports_formality");

        if(!cJSON_IsString(language) || !cJSON_IsString(name) || !cJSON_IsBool(supports_formality)) {
            strcpy(error_msg, "Invalid language format in response");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            cJSON_Delete(json);
            free(response);
            free(*languages);
            *languages = NULL;
            *count = 0;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return INVALID_FIELD_TYPE;
        }

        strncpy((*languages)[i].language, language->valuestring, sizeof((*languages)[i].language) - 1);
        (*languages)[i].language[sizeof((*languages)[i].language) - 1] = '\0';

        strncpy((*languages)[i].name, name->valuestring, sizeof((*languages)[i].name) - 1);
        (*languages)[i].name[sizeof((*languages)[i].name) - 1] = '\0';

        (*languages)[i].supports_formality = cJSON_IsTrue(supports_formality);
    }

    cJSON_Delete(json);
    free(response);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    log4c_category_log(log, LOG4C_PRIORITY_INFO, "Languages retrieved successfully (%d)\n", *count);

    return SUCCESS;
}

int call_translate(const char *original_msg, TargetLanguage * source_language, TargetLanguage *target_language, char *error_msg, char *translated_msg, size_t translated_msg_size) {
    log4c_category_t *log = initialize_log4c_category("call_translate");
    int res = SUCCESS;

    if(original_msg == NULL || target_language == NULL || translated_msg == NULL) {
        strcpy(error_msg, "Invalid parameters");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        return FAILURE;
    }

    CURL *curl = NULL;
    CURLcode curl_res;
    char *response = NULL;
    cJSON *request_json = NULL;
    cJSON *text_array = NULL;
    char *post_data = NULL;
    struct curl_slist *headers = NULL;

    if (!initialized) {
        strcpy(error_msg, "Deepl API not initialized");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        return FAILURE;
    }

    // Preparation of the URL
    char url[384];
    snprintf(url, sizeof(url), "%s/translate", deeplAPIConfig.base_url);

    // Preparation of the JSON for the request
    request_json = cJSON_CreateObject();
    if (request_json == NULL) {
        strcpy(error_msg, "Error creating JSON object request_json");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        return FAILURE;
    }

    // Add text as array
    text_array = cJSON_AddArrayToObject(request_json, "text");
    if (text_array == NULL) {
        strcpy(error_msg, "Error adding text array to JSON object");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        cJSON_Delete(request_json);
        return FAILURE;
    }

    cJSON_AddItemToArray(text_array, cJSON_CreateString(original_msg));

    // Add target language and source language
    cJSON_AddStringToObject(request_json, "source_lang", source_language->language);
    cJSON_AddStringToObject(request_json, "target_lang", target_language->language);
    
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Request JSON: %s\n", cJSON_Print(request_json));

    // Conversione del JSON in stringa
    post_data = cJSON_Print(request_json);
    if (post_data == NULL) {
        strcpy(error_msg, "Error converting JSON to string");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        cJSON_Delete(request_json);
        return FAILURE;
    }

    // Initialization of the CURL call
    curl = curl_easy_init();

    if (!curl) {
        strcpy(error_msg, "Failed to initialize CURL");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        free(post_data);
        cJSON_Delete(request_json);
        return FAILURE;
    }

    // Preparation of the headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: DeepL-Auth-Key %s", deeplAPIConfig.api_key);
    headers = curl_slist_append(headers, auth_header);

    // CURL configuration
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    int cnt = 5;
    int baseDelay = 1000000; // 1 secondo in microsecondi
    int sleepDelay = baseDelay;


    do {

        // Execute the call
        curl_res = curl_easy_perform(curl);

        if (curl_res != CURLE_OK) {
            sprintf(error_msg, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_res));
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = FAILURE;
            --cnt;
        } else if (response == NULL) {
            strcpy(error_msg, "No response received from server");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = FAILURE;
            --cnt;
        } else {
            // Parse the JSON response
            cnt = 0;
            cJSON *json = cJSON_Parse(response);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response JSON: %s\n", response);
            if (json == NULL) {
                strcpy(error_msg, "Failed to parse JSON response");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_JSON;
            } else {
                // Extract the translation
                cJSON *translations = cJSON_GetObjectItem(json, "translations");
                if (cJSON_IsArray(translations) && cJSON_GetArraySize(translations) > 0) {
                    cJSON *translation = cJSON_GetArrayItem(translations, 0);
                    cJSON *text = cJSON_GetObjectItem(translation, "text");

                    if (cJSON_IsString(text) && text->valuestring != NULL) {
                        strncpy(translated_msg, text->valuestring, translated_msg_size - 1);
                        translated_msg[translated_msg_size - 1] = '\0';
                        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Translation successful: %s\n", translated_msg);
                    } else {
                        strcpy(error_msg, "Text field not found or invalid in response");
                        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                        res = INVALID_FIELD_TYPE;
                    }
                } else {
                    strcpy(error_msg, "No translations found in response");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                    res = INVALID_FORMAT;
                }
                cJSON_Delete(json);
            }
        }

        if(cnt > 0) {
            usleep(sleepDelay);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Retrying... (%d attempts left, delay: %d ms)\n",
                               cnt, sleepDelay/1000);
            sleepDelay *= 2; // Backoff esponenziale: raddoppia il ritardo ad ogni tentativo
        }
    } while (cnt > 0);

    // Cleaning
    if (response != NULL) {
        free(response);
    }
    if (post_data != NULL) {
        free(post_data);
    }
    if (headers != NULL) {
        curl_slist_free_all(headers);
    }
    if (curl != NULL) {
        curl_easy_cleanup(curl);
    }
    if (request_json != NULL) {
        cJSON_Delete(request_json);
    }

    return res;
}
