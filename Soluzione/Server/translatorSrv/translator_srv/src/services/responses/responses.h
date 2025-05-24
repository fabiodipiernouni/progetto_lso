#ifndef AUTH_MGR_SRC_SERVICES_RESPONSES_RESPONSES_H_
#define AUTH_MGR_SRC_SERVICES_RESPONSES_RESPONSES_H_

#include "../../utils/rest_client.h"

#define STATUS_OK "OK"
#define STATUS_ERROR "ERROR"
#define STATUS_WRONG_LANGUAGE_CODE "WRONG_LANGUAGE_CODE"
#define STATUS_MISSING_FIELD "MISSING_FIELD"
#define STATUS_EMPTY_FIELD "EMPTY_FIELD"
#define STATUS_INVALID_FIELD_TYPE "INVALID_FIELD_TYPE"

char* create_simple_response(const char* esito, const char* message);
char* create_translate_response(const char *language_code_from, const char *language_code_to, const char *original_msg, const char *translated_msg);
char* create_supported_languages_response(const TargetLanguage *languages, int count);
void free_response(char** response);

#endif //AUTH_MGR_SRC_SERVICES_RESPONSES_RESPONSES_H_
