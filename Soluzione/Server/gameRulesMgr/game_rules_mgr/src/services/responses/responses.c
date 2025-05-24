#include "responses.h"
#include "../../utils/helper.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <log4c.h>
#include "../../entities/utente.h"
#include "../../entities/stanza.h"
#include "../rooms/rooms.h"
#include "../users/users.h"

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

cJSON * create_user_object(const Utente *user) {
    cJSON *user_obj = cJSON_CreateObject();
    if (user_obj == NULL) {
        return NULL;
    }

    cJSON_AddNumberToObject(user_obj, "id_utente", user->id_utente);
    cJSON_AddStringToObject(user_obj, "nome", user->nome);
    cJSON_AddStringToObject(user_obj, "cognome", user->cognome);
    cJSON_AddStringToObject(user_obj, "email", user->email);
    cJSON_AddStringToObject(user_obj, "codice_lingua", user->codice_lingua);

    return user_obj;
}

cJSON * create_player_object(AppContext* app_ctx, const Giocatore *player) {
    cJSON *player_obj = cJSON_CreateObject();
    if (player_obj == NULL) {
        return NULL;
    }

    if(player->indice_utente == -1) {
        cJSON_AddNullToObject(player_obj, "utente");
    } else {
        cJSON_AddItemToObject(player_obj, "utente", create_user_object(&app_ctx->users[player->indice_utente]));
    }
    cJSON_AddStringToObject(player_obj, "messaggio", player->messaggio);

    if(player->stepNumber == -1) {
        cJSON_AddNullToObject(player_obj, "stepNumber");
    } else {
        cJSON_AddNumberToObject(player_obj, "stepNumber", player->stepNumber);
    }
    
    return player_obj;
}

cJSON * create_room_object(AppContext* app_ctx, const Stanza *room) {
    log4c_category_t* log = initialize_log4c_category("create_room_object");
    log_room(room, log);

    cJSON *room_obj = cJSON_CreateObject();
    if (room_obj == NULL) {
        return NULL;
    }

    cJSON_AddNumberToObject(room_obj, "id_stanza", room->indice_stanza);
    cJSON_AddItemToObject(room_obj, "utente_host", create_user_object(&app_ctx->users[room->giocatori[0].indice_utente]));
    cJSON_AddStringToObject(room_obj, "nome", room->nome);
    char ordine_gioco_string[25];
    get_ordine_gioco_string(room->ordine_gioco, ordine_gioco_string);
    cJSON_AddStringToObject(room_obj, "ordine_gioco", ordine_gioco_string);
    char stato_gioco_string[25];
    get_stato_gioco_string(room->stato_gioco, stato_gioco_string);
    cJSON_AddStringToObject(room_obj, "stato_gioco", stato_gioco_string);
    cJSON_AddNumberToObject(room_obj, "numero_giocatori", calculate_players_number(app_ctx, room->indice_stanza));

    cJSON *users_array = cJSON_CreateArray();
    for (int i = 0; i < MAX_GIOCATORI; i++) {
        cJSON_AddItemToArray(users_array, create_player_object(app_ctx, &room->giocatori[i]));
    }
    cJSON_AddItemToObject(room_obj, "giocatori", users_array);
    
    int indice_giocatore_corrente = calculate_next_player(app_ctx, room->indice_stanza);
    if(room->stato_gioco != GIOCO_IN_CORSO || indice_giocatore_corrente == -1)
        cJSON_AddNullToObject(room_obj, "giocatore_corrente");
    else
        cJSON_AddItemToObject(room_obj, "giocatore_corrente", create_user_object(&app_ctx->users[room->giocatori[indice_giocatore_corrente].indice_utente]));

    cJSON *queue_array = cJSON_CreateArray();
    for (int i = 0; i < MAX_CODA; i++) {
        if(room->coda[i] != -1) {
            cJSON_AddItemToArray(queue_array, create_user_object(&app_ctx->users[room->coda[i]]));
        }
    }
    cJSON_AddItemToObject(room_obj, "coda", queue_array);

    return room_obj;
}

char* create_create_room_response(AppContext* app_ctx, int indice_stanza) {
    log4c_category_t* log = initialize_log4c_category("create_room_response");

    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);
    cJSON_AddItemToObject(root, "payload", create_room_object(app_ctx, &app_ctx->rooms[indice_stanza]));

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Response created: %s", response_str);
    return response_str;
}

char* create_get_rooms_response(AppContext* app_ctx) {
    log4c_category_t* log = initialize_log4c_category("get_rooms_response");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Creating get_rooms_response");

    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    cJSON *rooms_array = cJSON_CreateArray();
    for (int i = 0; i < MAX_ROOMS; i++) {
        if(app_ctx->rooms[i].indice_stanza != -1) {
            cJSON *room_obj = create_room_object(app_ctx, &app_ctx->rooms[i]);
            cJSON_AddItemToArray(rooms_array, room_obj);
        }
    }

    cJSON_AddItemToObject(root, "payload", rooms_array);

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return response_str;
}

char* create_get_room_response(AppContext* app_ctx, int indice_stanza) {

    log4c_category_t* log = initialize_log4c_category("get_room_response");
    log_room(&app_ctx->rooms[indice_stanza], log);

    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);
    cJSON_AddItemToObject(root, "payload", create_room_object(app_ctx, &app_ctx->rooms[indice_stanza]));

    char *response_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return response_str;
}

char* create_get_user_response(AppContext* app_ctx, int id_utente) {
    log4c_category_t* log = initialize_log4c_category("get_user_response");

    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Unable to create JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddStringToObject(root, "esito", STATUS_OK);

    if(id_utente == -1)
        cJSON_AddNullToObject(root, "payload");
    else
        cJSON_AddItemToObject(root, "payload", create_user_object(&app_ctx->users[id_utente]));

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