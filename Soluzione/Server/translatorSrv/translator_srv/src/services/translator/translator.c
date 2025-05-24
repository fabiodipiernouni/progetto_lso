#include <stdio.h>
#include <log4c.h>
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../utils/rest_client.h"

int translate(const char *original_msg, char *error_msg, char *language_code_from, char *language_code_to, char *translated_msg) {
    log4c_category_t *log = initialize_log4c_category("translate");
    if(original_msg == NULL || language_code_from == NULL || language_code_to == NULL || translated_msg == NULL) {
        strcpy(error_msg, "Invalid parameters");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        return FAILURE;
    }

    int res = SUCCESS;

    TargetLanguage source_language;
    memset(&source_language, 0, sizeof(TargetLanguage));
    if(strlen(language_code_from) > 2) language_code_from[2] = '\0';
    strcpy(source_language.language, language_code_from);

    TargetLanguage target_language;
    memset(&target_language, 0, sizeof(TargetLanguage));
    strcpy(target_language.language, language_code_to);

    res = call_translate(original_msg, &source_language, &target_language, error_msg, translated_msg, MAX_MESSAGE_SIZE);

    if(res == SUCCESS)
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Translation successful: %s\n", translated_msg);
    else {
        sprintf(error_msg, "Translation failed: %d", res);
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
    }

    return res;
}