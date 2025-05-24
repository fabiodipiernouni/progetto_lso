#include <stdio.h>
#include <stdlib.h>
#include <log4c.h>
#include <cjson/cJSON.h>
#include <string.h>
#include "payload_validators.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../entities/lingua.h"
#include "../../repository/lingue/lingue.h"

int validate_password(const char* password) {
    if (password == NULL) {
        return MISSING_FIELD;
    }

    // Check minimum length
    size_t len = strlen(password);
    if (len < 8) {
        return PASSWORD_TOO_SHORT;
    }

    int has_lowercase = 0;
    int has_uppercase = 0;
    int has_digit = 0;

    // Check character by character
    for (size_t i = 0; i < len; i++) {
        char c = password[i];
        if (c >= 'a' && c <= 'z') {
            has_lowercase = 1;
        } else if (c >= 'A' && c <= 'Z') {
            has_uppercase = 1;
        } else if (c >= '0' && c <= '9') {
            has_digit = 1;
        }
    }

    if (!has_lowercase) {
        return PASSWORD_NO_LOWERCASE; // No lowercase letter
    }
    if (!has_uppercase) {
        return PASSWORD_NO_UPPERCASE; // No uppercase letter
    }
    if (!has_digit) {
        return PASSWORD_NO_DIGIT; // No digit
    }

    return SUCCESS;
}

int validate_email(const char* email) {
    if (email == NULL) {
        return MISSING_FIELD;
    }

    size_t len = strlen(email);
    if (len == 0 || len > 254) { // RFC 5321 limits email length
        return MISSING_FIELD;
    }

    // Find @ symbol
    const char* at = strchr(email, '@');
    if (at == NULL) {
        return -3; // No @ symbol
    }

    // Check for multiple @ symbols
    if (strchr(at + 1, '@') != NULL) {
        return -4; // Multiple @ symbols
    }

    // Check local part (before @)
    if (at == email) {
        return -5; // Empty local part
    }

    // Check domain part (after @)
    size_t domain_len = len - (at - email + 1);
    if (domain_len < 3) { // At minimum x.y
        return -6; // Domain part too short
    }

    // Check for at least one dot in domain
    const char* dot = strchr(at + 1, '.');
    if (dot == NULL || dot == at + 1) {
        return -7; // No dot in domain or dot immediately after @
    }

    // Check for at least one character after the last dot
    const char* last_dot = strrchr(email, '.');
    if (last_dot == NULL || *(last_dot + 1) == '\0') {
        return -8; // No characters after dot
    }

    return SUCCESS; // Valid email
}

int validate_register_user_payload(const char *payload, char*error_msg, utente* user) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_register_user_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_JSON;
    }

    // Check email
    if (res == SUCCESS) {
        cJSON *email_json = cJSON_GetObjectItemCaseSensitive(json, "email");

        if (email_json == NULL) {
            strcpy(error_msg, "Email field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (email_json->type != cJSON_String) {
            strcpy(error_msg, "Email field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(email_json->valuestring) == 0) {
            strcpy(error_msg, "Email field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else if ((res = validate_email(email_json->valuestring) != SUCCESS)) {
            strcpy(error_msg, "Email format is invalid");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FORMAT;
        } else
            strcpy(user->email, email_json->valuestring);
    }

    // Check name
    if(res == SUCCESS) {
        cJSON *nome_json = cJSON_GetObjectItemCaseSensitive(json, "nome");

        if (nome_json == NULL) {
            strcpy(error_msg, "Name field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (nome_json->type != cJSON_String) {
            strcpy(error_msg, "Name field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(nome_json->valuestring) == 0) {
            strcpy(error_msg, "Name field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else
            strcpy(user->nome, nome_json->valuestring);
    }

    // Check surname
    if(res == SUCCESS) {
        cJSON *cognome_json = cJSON_GetObjectItemCaseSensitive(json, "cognome");

        if (cognome_json == NULL || strlen(cognome_json->valuestring) == 0) {
            strcpy(error_msg, "Surname field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (cognome_json->type != cJSON_String) {
            strcpy(error_msg, "Surname field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        }
        else if (strlen(cognome_json->valuestring) == 0) {
            strcpy(error_msg, "Surname field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        }
        else
            strcpy(user->cognome, cognome_json->valuestring);
    }

    // Check password
    if(res == SUCCESS) {
        cJSON *password_json = cJSON_GetObjectItemCaseSensitive(json, "password");

        if (password_json == NULL) {
            strcpy(error_msg, "Password field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (password_json->type != cJSON_String) {
            strcpy(error_msg, "Password field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(password_json->valuestring) == 0) {
            strcpy(error_msg, "Password field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else if ((res = validate_password(password_json->valuestring)) != SUCCESS) {
            char cause[64];
            get_invalid_password_msg(res, cause);
            sprintf(error_msg, "Invalid password - %s", cause);

            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s (res %d)\n", error_msg, res);
            // res was set by validate_password, no need to do anything
        } else
            strcpy(user->password, password_json->valuestring);
    }

    // Check language
    if(res == SUCCESS) {
        cJSON *lingua_json = cJSON_GetObjectItemCaseSensitive(json, "lingua");

        if (lingua_json == NULL) {
            strcpy(error_msg, "Language field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (lingua_json->type != cJSON_String) {
            strcpy(error_msg, "Language field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(lingua_json->valuestring) == 0) {
            strcpy(error_msg, "Language field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else {
            char descrizione[128];
            res = get_descrizione_lingua(lingua_json->valuestring, descrizione);
            if (res != SUCCESS) {
                strcpy(error_msg, "Invalid language");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                // res was set by get_descrizione_lingua, no need to do anything
            } else
                strcpy(user->codice_lingua, lingua_json->valuestring);
        }
    }

    cJSON_Delete(json);
    return res;
}

int validate_get_language_payload(const char *payload, char*error_msg, lingua* lang) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_get_language_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_JSON;
    }

    // Check language
    cJSON *codice_lingua_json = cJSON_GetObjectItemCaseSensitive(json, "codice_lingua");

    if (codice_lingua_json == NULL) {
        strcpy(error_msg, "Language field not found");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = MISSING_FIELD;
    } else if (codice_lingua_json->type != cJSON_String) {
        strcpy(error_msg, "Language field is not a string");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_FIELD_TYPE;
    } else if (strlen(codice_lingua_json->valuestring) == 0) {
        strcpy(error_msg, "Language field is empty");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = EMPTY_FIELD;
    } else
        strcpy(lang->codice_lingua, codice_lingua_json->valuestring);

    cJSON_Delete(json);
    return res;
}

int validate_update_user_payload(const char *payload, char*error_msg, char *email, char *new_email, char *new_password, char *new_lang, char *new_nome, char *new_cognome) {
    int res = SUCCESS;
    int case_managed = 0;
    log4c_category_t* log = initialize_log4c_category("validate_update_user_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_JSON;
    }

    // Check email
    if(res == SUCCESS) {
        cJSON *json_email = cJSON_GetObjectItem(json, "email");

        if (json_email == NULL) {
            strcpy(error_msg, "Invalid payload, email field not valorized.");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (json_email->type != cJSON_String) {
            strcpy(error_msg, "Invalid payload, email field not a string.");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if ((res = validate_email(json_email->valuestring) != SUCCESS)) {
            strcpy(error_msg, "Invalid email");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FORMAT;
        } else
            strcpy(email, json_email->valuestring);
    }

    // Check new email, password and language
    if(res == SUCCESS) {
        cJSON *json_new_email = cJSON_GetObjectItem(json, "new_email");

        if (json_new_email != NULL) {
            case_managed++;
            if (json_new_email->type != cJSON_String) {
                strcpy(error_msg, "Invalid payload, new email field not a string.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_FIELD_TYPE;
            } else if ((res = validate_email(json_new_email->valuestring) != SUCCESS)) {
                strcpy(error_msg, "Invalid new email");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            } else {
                strcpy(new_email, json_new_email->valuestring);
            }
        }
    }

    // Check old password and new password
    if(res == SUCCESS) {
        cJSON *json_new_password = cJSON_GetObjectItem(json, "new_password");
        if (json_new_password != NULL) {
            case_managed++;
            if (json_new_password->type != cJSON_String) {
                strcpy(error_msg, "Invalid payload, new password field not a string.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_FIELD_TYPE;
            } else if ((res = validate_password(json_new_password->valuestring) != SUCCESS)) {
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Invalid new password (res = %d).\n", res);
                get_invalid_password_msg(res, error_msg);
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = WRONG_PASSWORD;
            } else {
                strcpy(new_password, json_new_password->valuestring);
            }
        }
    }

    if(res == SUCCESS) {
        cJSON *json_new_lang = cJSON_GetObjectItem(json, "new_lang");

        if (json_new_lang != NULL) {
            case_managed++;
            if (json_new_lang->type != cJSON_String) {
                strcpy(error_msg, "Invalid payload, new language field not a string.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_FIELD_TYPE;
            } else {
                char descrizione[128] = {0};
                if ((res = get_descrizione_lingua(json_new_lang->valuestring, descrizione)) != SUCCESS) {;
                    strcpy(error_msg, "Invalid new language");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                } else {
                    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Descrizione lingua: %s\n", descrizione);
                    strcpy(new_lang, json_new_lang->valuestring);
                }
            }
        }
    }

    // Check new name
    if(res == SUCCESS) {
        cJSON *json_new_nome = cJSON_GetObjectItem(json, "new_nome");

        if (json_new_nome != NULL) {
            case_managed++;
            if (json_new_nome->type != cJSON_String) {
                strcpy(error_msg, "Invalid payload, new name field not a string.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_FIELD_TYPE;
            } else if (strlen(json_new_nome->valuestring) == 0) {
                strcpy(error_msg, "Invalid payload, new name field is empty.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = EMPTY_FIELD;
            } else {
                strcpy(new_nome, json_new_nome->valuestring);
            }
        }
    }

    // Check new surname
    if(res == SUCCESS) {
        cJSON *json_new_cognome = cJSON_GetObjectItem(json, "new_cognome");

        if (json_new_cognome != NULL) {
            case_managed++;
            if (json_new_cognome->type != cJSON_String) {
                strcpy(error_msg, "Invalid payload, new surname field not a string.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_FIELD_TYPE;
            } else if (strlen(json_new_cognome->valuestring) == 0) {
                strcpy(error_msg, "Invalid payload, new surname field is empty.");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = EMPTY_FIELD;
            } else {
                strcpy(new_cognome, json_new_cognome->valuestring);
            }
        }
    }

    if(res == SUCCESS && case_managed == 0) {
        strcpy(error_msg, "Invalid payload, no fields to update.");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = MISSING_FIELD;
    }

    cJSON_Delete(json);
    return res;
}

int validate_get_user_payload(const char *payload, char*error_msg, char* email, long* user_id) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_get_user_payload");

    cJSON *json = cJSON_Parse(payload);

    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = FAILURE;
    }

    if(res == SUCCESS) {
        cJSON *email_json = cJSON_GetObjectItemCaseSensitive(json, "email");
        cJSON *user_id_json = cJSON_GetObjectItemCaseSensitive(json, "user_id");
        if (email_json == NULL && user_id_json == NULL) {
            strcpy(error_msg, "Email field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else {
            if (user_id_json != NULL) {
                if (user_id_json->type != cJSON_Number) {
                    strcpy(error_msg, "User ID field is not a number");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                    res = INVALID_FIELD_TYPE;
                } else if (user_id_json->valueint <= 0) {
                    strcpy(error_msg, "User ID field is invalid");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                    res = INVALID_FIELD_TYPE;
                } else {
                    *user_id = user_id_json->valueint;
                }
            }

            if (email_json != NULL) {
                if (email_json->type != cJSON_String) {
                    strcpy(error_msg, "Email field is not a string");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                    res = INVALID_FIELD_TYPE;
                } else if (strlen(email_json->valuestring) == 0) {
                    strcpy(error_msg, "Email field is empty");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                    res = EMPTY_FIELD;
                } else if ((res = validate_email(email_json->valuestring) != SUCCESS)) {
                    strcpy(error_msg, "Invalid email");
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                    // res was set by validate_email, no need to do anything
                } else {
                    strcpy(email, email_json->valuestring);
                }
            }
        }
    }

    cJSON_Delete(json);
    return res;
}

int validate_login_payload(const char *payload, int*id_session, char *email, char *password, char*error_msg) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_login_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = FAILURE;
    }

    if (res == SUCCESS) {
        cJSON *email_json = cJSON_GetObjectItemCaseSensitive(json, "email");
        if (email_json == NULL) {
            strcpy(error_msg, "Email field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (email_json->type != cJSON_String) {
            strcpy(error_msg, "Email field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(email_json->valuestring) == 0) {
            strcpy(error_msg, "Email field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else if ((res = validate_email(email_json->valuestring) != SUCCESS)) {
            strcpy(error_msg, "Invalid email");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            // res was set by validate_email, no need to do anything
        } else
            strcpy(email, email_json->valuestring);
    }

    if (res == SUCCESS) {
        cJSON *password_json = cJSON_GetObjectItemCaseSensitive(json, "password");
        if (password_json == NULL) {
            strcpy(error_msg, "Password field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (password_json->type != cJSON_String) {
            strcpy(error_msg, "Password field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(password_json->valuestring) == 0) {
            strcpy(error_msg, "Password field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else
            strcpy(password, password_json->valuestring);
    }

    cJSON_Delete(json);
    return res;
}
