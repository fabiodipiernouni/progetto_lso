#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <string.h>

#include <log4c.h>
#include "users.h"

#include "../../utils/mapping.h"
#include "../../utils/helper.h"

#include "../socket_mgr.h"
#include "../rooms/rooms.h"

void reset_user(AppContext* app_ctx, int indice_utente) {
    app_ctx->users[indice_utente].indice_utente = indice_utente;
    app_ctx->users[indice_utente].id_utente = -1; // Mark the user as empty
    app_ctx->users[indice_utente].nome[0] = '\0'; // Clear the user name
    app_ctx->users[indice_utente].cognome[0] = '\0'; // Clear the user surname
    app_ctx->users[indice_utente].email[0] = '\0'; // Clear the user email
    app_ctx->users[indice_utente].codice_lingua[0] = '\0'; // Clear the user language code
}

void initialize_users(AppContext* app_ctx) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        reset_user(app_ctx, i);
    }
}

int check_same_user_id_already_set(AppContext* app_ctx, const int id_utente, const int indice_client) {

    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("check_same_user_id_already_set");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (app_ctx->users[i].id_utente != -1 && app_ctx->users[i].id_utente == app_ctx->users[indice_client].id_utente && i != indice_client) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User id %d already set by another client %d\n", app_ctx->users[indice_client].id_utente, i);
            res = USER_SET_BY_ANOTHER_CLIENT;
            reset_user(app_ctx, indice_client); 
            break;
        }
    }


    return res;
}

bool has_user_set_id(AppContext* app_ctx, const int indice_utente) {
    return app_ctx->users[indice_utente].id_utente != -1;
}

int get_room_by_user_index(AppContext* app_ctx, const int indice_utente) {

    for (int i = 0; i < MAX_ROOMS; i++) {
        for(int j = 0; j < MAX_GIOCATORI; j++) {
            if (app_ctx->rooms[i].giocatori[j].indice_utente == indice_utente) {
                return i;
            }
        }
        for(int j = 0; j < MAX_CODA; j++) {
            if (app_ctx->rooms[i].coda[j] == indice_utente) {
                return i;
            }
        }
    }

    return -1;
}

int disconnect_user(AppContext* app_ctx, const int indice_utente) {
    int res = SUCCESS;
    log4c_category_t* log = initialize_log4c_category("disconnect_user");

    if(has_user_set_id(app_ctx, indice_utente)) {
        int indice_stanza = get_room_by_user_index(app_ctx, indice_utente); // Reset the room for the user
        if (indice_stanza != -1) {
            leave_room(app_ctx, indice_utente, indice_stanza); // Leave the room if the user is in one
        }
        reset_user(app_ctx, indice_utente);

        log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %d logged out\n", indice_utente);
    }

    return res;
}