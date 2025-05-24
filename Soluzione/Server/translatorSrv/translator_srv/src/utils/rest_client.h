#ifndef TRANSLATOR_SRV_SRC_UTILS_REST_CLIENT_H_
#define TRANSLATOR_SRV_SRC_UTILS_REST_CLIENT_H_

#include <stdlib.h>
#include <stdbool.h>

typedef struct __DEEPL_API__ {
    char api_key[64];
    char base_url[256];
} DeeplAPI;

typedef struct __TARGET_LANGUAGE__ {
    char language[6];
    char name[128];
    bool supports_formality;
} TargetLanguage;


int init_deepl_api_config(const char *config_file);
const DeeplAPI get_deepl_api_config();

int get_supported_languages(char* error_msg, TargetLanguage **languages, int *count);

int call_translate(const char *original_msg, TargetLanguage * source_language, TargetLanguage *target_language, char *error_msg, char *translated_msg, size_t max_translation_size);

#endif //TRANSLATOR_SRV_SRC_UTILS_REST_CLIENT_H_
