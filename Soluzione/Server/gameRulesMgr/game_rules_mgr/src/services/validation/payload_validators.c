#include <stdio.h>
#include <stdlib.h>
#include <log4c.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <ctype.h>
#include "payload_validators.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../entities/utente.h"
#include "../../entities/stanza.h"
#include "../../services/rooms/rooms.h"
#include "../../services/users/users.h"

int validate_ordine_gioco(const char *ordine_gioco_str, ordine_gioco_t *ordine_gioco) {
    if (strcmp(ordine_gioco_str, "ORARIO") == 0) {
        *ordine_gioco = ORARIO;
        return SUCCESS;
    } else if (strcmp(ordine_gioco_str, "ANTIORARIO") == 0) {
        *ordine_gioco = ANTIORARIO;
        return SUCCESS;
    } else {
        return INVALID_FORMAT;
    }
}

int validate_nome_stanza(AppContext* app_ctx, const char *room_name) {
    if (strlen(room_name) == 0) {
        return EMPTY_FIELD;
    }
    if (strlen(room_name) > MAX_NOME_STANZA) {
        return INVALID_FIELD_LENGTH;
    }
    for (int i = 0; i < strlen(room_name); i++) {
        if (!isalnum(room_name[i]) && room_name[i] != ' ') {
            return INVALID_FIELD_TYPE;
        }
    }
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (app_ctx->rooms[i].indice_stanza != -1 && strcasecmp(app_ctx->rooms[i].nome, room_name) == 0) {
            return ROOM_NAME_TAKEN;
        }
    }
    return SUCCESS;
}

int validate_create_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, char *room_name, ordine_gioco_t *ordine_gioco) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_create_room_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = FAILURE;
    }

    cJSON *room_name_json = cJSON_GetObjectItemCaseSensitive(json, "nome_stanza");
    if (room_name_json == NULL) {
        strcpy(error_msg, "Room name field not found");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = MISSING_FIELD;
    } else if (room_name_json->type != cJSON_String) {
        strcpy(error_msg, "Room name field is not a string");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_FIELD_TYPE;
    } else if (strlen(room_name_json->valuestring) == 0) {
        strcpy(error_msg, "Room name field is empty");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = EMPTY_FIELD;
    } else {
        strcpy(room_name, room_name_json->valuestring);
        res = validate_nome_stanza(app_ctx, room_name);
        if (res != SUCCESS) {

            get_invalid_room_name_msg(res, error_msg);
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s", error_msg);
            res = INVALID_FORMAT;
        }
    }

    if (res == SUCCESS) {
        cJSON *ordine_gioco_json = cJSON_GetObjectItemCaseSensitive(json, "ordine_gioco");
        if (ordine_gioco_json == NULL) {
            strcpy(error_msg, "Game order field not found");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = MISSING_FIELD;
        } else if (ordine_gioco_json->type != cJSON_String) {
            strcpy(error_msg, "Game order field is not a string");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = INVALID_FIELD_TYPE;
        } else if (strlen(ordine_gioco_json->valuestring) == 0) {
            strcpy(error_msg, "Game order field is empty");
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
            res = EMPTY_FIELD;
        } else {
            res = validate_ordine_gioco(ordine_gioco_json->valuestring, ordine_gioco);
            if (res != SUCCESS) {
                strcpy(error_msg, "Invalid game order value, must be ORARIO or ANTIORARIO");
                log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
                res = INVALID_FORMAT;
            }
        }
    }

    cJSON_Delete(json);
    return res;
}

int validate_room_id_payload(AppContext* app_ctx, const char *payload, char*error_msg, int *indice_stanza) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_room_id_payload");

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "%s\n", payload);


    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = FAILURE;
    }

    cJSON *room_id_json = cJSON_GetObjectItemCaseSensitive(json, "id_stanza");
    if (room_id_json == NULL) {
        strcpy(error_msg, "Room ID field not found");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = MISSING_FIELD;
    } else if (room_id_json->type != cJSON_Number) {
        strcpy(error_msg, "Room ID field is not a number");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_FIELD_TYPE;
    } else if (room_id_json->valueint < 0 || room_id_json->valueint >= MAX_ROOMS) {
        strcpy(error_msg, "Room ID out of range");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_FIELD_VALUE;
    } else if (app_ctx->rooms[room_id_json->valueint].indice_stanza == -1) {
        strcpy(error_msg, "Room ID not found");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = ROOM_NOT_FOUND;
    } else {
        *indice_stanza = room_id_json->valueint;
    }

    cJSON_Delete(json);
    return res;
}

int validate_join_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza) {
    return validate_room_id_payload(app_ctx, payload, error_msg, indice_stanza);
}

int validate_leave_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza) {
    return validate_room_id_payload(app_ctx, payload, error_msg, indice_stanza);
}

int validate_get_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza) {
    return validate_room_id_payload(app_ctx, payload, error_msg, indice_stanza);
}

int validate_start_game_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza) {
    return validate_room_id_payload(app_ctx, payload, error_msg, indice_stanza);
}

int validate_send_message_payload(const char *payload, char*error_msg, char *message) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("validate_send_message_payload");

    cJSON *json = cJSON_Parse(payload);
    if (json == NULL) {
        strcpy(error_msg, "Failed to parse JSON");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = FAILURE;
    }

    cJSON *message_json = cJSON_GetObjectItemCaseSensitive(json, "messaggio");
    if (message_json == NULL) {
        strcpy(error_msg, "Message field not found");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = MISSING_FIELD;
    } else if (message_json->type != cJSON_String) {
        strcpy(error_msg, "Message field is not a string");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n",
            error_msg);
        res = INVALID_FIELD_TYPE;
    } else if (strlen(message_json->valuestring) == 0) {
        strcpy(error_msg, "Message field is empty");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = EMPTY_FIELD;
    } else if (strlen(message_json->valuestring) > MAX_LUNGHEZZA_MESSAGGIO) {
        strcpy(error_msg, "Message field is too long");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s\n", error_msg);
        res = INVALID_FIELD_LENGTH;
    } else {
        strcpy(message, message_json->valuestring);
    }
    cJSON_Delete(json);
    return res;
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

int validate_set_user_payload(const char *payload, char*error_msg, char* email) {
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
            //res valorizzato da validate_email, non serve fare nulla
        } else
            strcpy(email, email_json->valuestring);
    }

    cJSON_Delete(json);
    return res;
}