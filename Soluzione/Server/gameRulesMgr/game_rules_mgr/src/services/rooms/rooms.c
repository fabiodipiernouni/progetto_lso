#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <pthread.h>

#include <log4c.h>
#include "rooms.h"

#include "../../utils/mapping.h"
#include "../../utils/helper.h"

#include "../users/users.h"

// Function to clean up all individual room mutexes
void cleanup_all_room_mutexes(AppContext* app_ctx) {
    // No need to lock rooms_mutex here as this is called during shutdown
    // when no other threads should be accessing rooms.
    for (int i = 0; i < MAX_ROOMS; i++) {
        // Assuming all room mutexes were initialized if rooms_mutex was.
        // reset_room initializes stanza_mutex.
        // If app_ctx->rooms[i].indice_stanza != -1, it means it was active and has a mutex.
        // However, initialize_rooms calls reset_room for all, so all should have one.
        pthread_mutex_destroy(&app_ctx->rooms[i].stanza_mutex);
    }
}

void reset_player(AppContext* app_ctx, int indice_stanza, int indice_giocatore) {
    app_ctx->rooms[indice_stanza].giocatori[indice_giocatore].indice_utente = -1; // Mark the player as empty
    app_ctx->rooms[indice_stanza].giocatori[indice_giocatore].messaggio[0] = '\0'; // Clear the message'
    app_ctx->rooms[indice_stanza].giocatori[indice_giocatore].stepNumber = -1; // Reset the step
}

void reset_messages(AppContext* app_ctx, int room_id) {
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        app_ctx->rooms[room_id].giocatori[j].messaggio[0] = '\0'; // Clear the message
        app_ctx->rooms[room_id].giocatori[j].stepNumber = -1; // Reset the step
    }
}

void reset_room(AppContext* app_ctx, int room_id) {
    if (app_ctx->rooms[room_id].indice_stanza != -1) {
        pthread_mutex_destroy(&app_ctx->rooms[room_id].stanza_mutex);
    }

    app_ctx->rooms[room_id].indice_stanza = -1; // Mark the room as empty
    app_ctx->rooms[room_id].nome[0] = '\0'; // Clear the room name
    app_ctx->rooms[room_id].ordine_gioco = ORARIO; // Default to clockwise
    app_ctx->rooms[room_id].stato_gioco = IN_ATTESA_DI_GIOCATORI;
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        reset_player(app_ctx, room_id, j);
    }
    for (int j = 0; j < MAX_CODA; j++) {
        app_ctx->rooms[room_id].coda[j] = -1; // Mark the queue as empty
    }
    pthread_mutex_init(&app_ctx->rooms[room_id].stanza_mutex, NULL);
}

void initialize_rooms(AppContext* app_ctx) {
    log4c_category_t* log = initialize_log4c_category("initialize_rooms");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Initializing rooms\n");
    pthread_mutex_lock(&app_ctx->rooms_mutex);
    for (int i = 0; i < MAX_ROOMS; i++) {
        reset_room(app_ctx, i);
    }
    pthread_mutex_unlock(&app_ctx->rooms_mutex);
}

int get_last_available_slot(AppContext* app_ctx, int room_id) {
    for (int j = MAX_GIOCATORI-1; j >= 0; j--) {
        if (app_ctx->rooms[room_id].giocatori[j].indice_utente == -1) {
            return j;
        }
    }
    return -1; // No available slots
}

int get_first_available_slot(AppContext* app_ctx, int room_id) {
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        if (app_ctx->rooms[room_id].giocatori[j].indice_utente == -1) {
            return j;
        }
    }
    return -1; // No available slots
}

int calculate_index_player(AppContext* app_ctx, int room_id, int indice_utente) {
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        if (app_ctx->rooms[room_id].giocatori[j].indice_utente == indice_utente) {
            return j;
        }
    }
    return -1; // User not found in the room
}

void swap_player(AppContext* app_ctx, int room_id, int indice_giocatore_src, int indice_giocatore_dst) {
    Giocatore temp = app_ctx->rooms[room_id].giocatori[indice_giocatore_src];
    app_ctx->rooms[room_id].giocatori[indice_giocatore_src] = app_ctx->rooms[room_id].giocatori[indice_giocatore_dst];
    app_ctx->rooms[room_id].giocatori[indice_giocatore_dst] = temp;
}

int calculate_players_number(AppContext* app_ctx, const int room_id) {
    int count = 0;
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        if (app_ctx->rooms[room_id].giocatori[j].indice_utente != -1) {
            count++;
        }
    }
    return count;
}

int calculate_next_player(AppContext* app_ctx, int room_id) {

    // the first to talk is the host
    if(strlen(app_ctx->rooms[room_id].giocatori[0].messaggio) == 0) {
        return 0;
    }

    if (app_ctx->rooms[room_id].ordine_gioco == ORARIO) {
        // find the next player in a clockwise direction
        for (int j = 1; j < MAX_GIOCATORI; j++) {
            if (app_ctx->rooms[room_id].giocatori[j].indice_utente != -1 && strlen(app_ctx->rooms[room_id].giocatori[j].messaggio) == 0) {
                return j;
            }
        }
    }

    // find the next player in a counterclockwise direction
    for (int j = MAX_GIOCATORI-1; j > 0; j--) {
        if (app_ctx->rooms[room_id].giocatori[j].indice_utente != -1 && strlen(app_ctx->rooms[room_id].giocatori[j].messaggio) == 0) {
            return j;
        }
    }

    return -1; // No players available to play

}

void check_if_game_finished(AppContext* app_ctx, int room_id) {
    if(calculate_next_player(app_ctx, room_id) == -1) {
        app_ctx->rooms[room_id].stato_gioco = GIOCO_TERMINATO;
    }
}

int create_room(AppContext* app_ctx, int indice_utente_host, char *room_name, ordine_gioco_t ordine_gioco, int *room_id) {
    log4c_category_t* log = initialize_log4c_category("create_room");
    *room_id = -1;

    // check if user is already in room
    if(get_room_by_user_index(app_ctx, indice_utente_host) != -1) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User %d is already in a room\n", indice_utente_host);
        return USER_ALREADY_IN_ROOM;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Creating room %s with host %d\n", room_name, indice_utente_host);

    pthread_mutex_lock(&app_ctx->rooms_mutex);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (app_ctx->rooms[i].indice_stanza == -1) { // id -1 means the room is empty
            app_ctx->rooms[i].indice_stanza = i; // Assign the index as the room ID
            strncpy(app_ctx->rooms[i].nome, room_name, MAX_NOME_STANZA - 1);
            app_ctx->rooms[i].nome[MAX_NOME_STANZA - 1] = '\0'; // Ensure null termination
            app_ctx->rooms[i].ordine_gioco = ordine_gioco;
            app_ctx->rooms[i].stato_gioco = IN_ATTESA_DI_GIOCATORI;
            app_ctx->rooms[i].giocatori[0].indice_utente = indice_utente_host; // Assign the host to the first player slot
            *room_id = i;
            pthread_mutex_unlock(&app_ctx->rooms_mutex);
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Room %s created with ID %d by host %d\n", room_name, i, indice_utente_host);
            return SUCCESS;
        }
    }
    pthread_mutex_unlock(&app_ctx->rooms_mutex);
    return ROOM_LIMIT_REACHED;
}

int enqueue(AppContext* app_ctx, int indice_utente, int room_id) {
    log4c_category_t* log = initialize_log4c_category("enqueue");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d trying to join the queue for room %d\n", indice_utente, room_id);

    for (int j = 0; j < MAX_CODA; j++) {
        if (app_ctx->rooms[room_id].coda[j] == -1) {
            app_ctx->rooms[room_id].coda[j] = indice_utente;
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d enqueued for room %d at position %d\n", indice_utente, room_id, j);
            return SUCCESS;
        }
    }

    return QUEUE_FULL; // Queue is full
}

void shift_queue(AppContext* app_ctx, int room_id, int start) {
    // Shift the queue
    for (int k = start; k < MAX_CODA - 1; k++) {
        app_ctx->rooms[room_id].coda[k] = app_ctx->rooms[room_id].coda[k + 1];
    }
    app_ctx->rooms[room_id].coda[MAX_CODA - 1] = -1; // Mark the last slot as empty
}

int dequeue(AppContext* app_ctx, int room_id) {
    log4c_category_t* log = initialize_log4c_category("dequeue");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Dequeueing user from room %d\n", room_id);

    for (int j = 0; j < MAX_CODA; j++) {
        if (app_ctx->rooms[room_id].coda[j] != -1) {
            int user_id = app_ctx->rooms[room_id].coda[j];
            shift_queue(app_ctx, room_id, j); // Shift the queue to remove the user
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d dequeued from room %d\n", user_id, room_id);
            return user_id;
        }
    }

    return -1; // No users in the queue
}

int calculate_index_queue(AppContext* app_ctx, int room_id, int indice_utente) {
    for (int j = 0; j < MAX_CODA; j++) {
        if (app_ctx->rooms[room_id].coda[j] == indice_utente) {
            return j;
        }
    }
    return -1; // User not found in the queue
}

int remove_user_from_queue(AppContext* app_ctx, int room_id, int indice_utente) {
    log4c_category_t* log = initialize_log4c_category("remove_user_from_queue");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Removing user %d from the queue of room %d\n", indice_utente, room_id);

    int index = calculate_index_queue(app_ctx, room_id, indice_utente);
    if (index == -1) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User %d not found in the queue of room %d\n", indice_utente, room_id);
        return USER_NOT_IN_ROOM; // User not found in the queue
    }

    shift_queue(app_ctx, room_id, index); // Shift the queue to remove the user
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d removed from the queue of room %d\n", indice_utente, room_id);
    return SUCCESS;
}

int join_room(AppContext* app_ctx, int indice_utente, int room_id) {
    log4c_category_t* log = initialize_log4c_category("join_room");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d trying to join room %d\n", indice_utente, room_id);

    pthread_mutex_lock(&app_ctx->rooms_mutex);
    if (room_id < 0 || room_id >= MAX_ROOMS || app_ctx->rooms[room_id].indice_stanza == -1) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Room %d does not exist or is not active.\n", room_id);
        pthread_mutex_unlock(&app_ctx->rooms_mutex);
        return ROOM_NOT_FOUND;
    }

    pthread_mutex_lock(&app_ctx->rooms[room_id].stanza_mutex);
    pthread_mutex_unlock(&app_ctx->rooms_mutex);

    int indice_giocatore = calculate_index_player(app_ctx, room_id, indice_utente);
    if (indice_giocatore != -1) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User %d is already in room %d\n", indice_utente, room_id);
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return USER_ALREADY_IN_ROOM;
    }

    int slot = get_first_available_slot(app_ctx, room_id);
    if (slot == -1) {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Room %d is full, trying to enqueue user %d\n", room_id, indice_utente);
        if(enqueue(app_ctx, indice_utente, room_id) != SUCCESS) {
            pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
            return QUEUE_FULL;
        } else {
            pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
            return SUCCESS; // User enqueued
        }
    }

    app_ctx->rooms[room_id].giocatori[slot].indice_utente = indice_utente;
    pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
    return SUCCESS;
}

void update_steps_number(AppContext* app_ctx, int room_id, int stepNumber) {
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        if (app_ctx->rooms[room_id].giocatori[j].stepNumber > stepNumber) {
            app_ctx->rooms[room_id].giocatori[j].stepNumber--;
        }
    }
}

int leave_room(AppContext* app_ctx, int indice_utente, int room_id) {
    log4c_category_t* log = initialize_log4c_category("leave_room");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d trying to leave room %d\n", indice_utente, room_id);

    pthread_mutex_lock(&app_ctx->rooms_mutex);
    if (room_id < 0 || room_id >= MAX_ROOMS || app_ctx->rooms[room_id].indice_stanza == -1) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Room %d does not exist or is not active for leave operation.\n", room_id);
        pthread_mutex_unlock(&app_ctx->rooms_mutex);
        return ROOM_NOT_FOUND;
    }

    pthread_mutex_lock(&app_ctx->rooms[room_id].stanza_mutex);
    pthread_mutex_unlock(&app_ctx->rooms_mutex);

    int indice_giocatore = calculate_index_player(app_ctx, room_id, indice_utente);

    if (indice_giocatore == -1) {
        if (calculate_index_queue(app_ctx, room_id, indice_utente) != -1) {
            remove_user_from_queue(app_ctx, room_id, indice_utente);
            pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
            return SUCCESS; // User was in queue and removed
        }
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User %d not found in room %d or its queue.\n", indice_utente, room_id);
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return USER_NOT_IN_ROOM;
    }

    const int numero_giocatori = calculate_players_number(app_ctx, room_id);

    if(numero_giocatori - 1 == 0) {
        reset_room(app_ctx, room_id); // Reset the room if it becomes empty
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Room %d is now empty and has been reset.\n", room_id);
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return SUCCESS;
    }

    if (app_ctx->rooms[room_id].giocatori[0].indice_utente == indice_utente) {
        // Host is leaving
        if (numero_giocatori > 1) {
            // Promote the next player to host
            app_ctx->rooms[room_id].giocatori[0] = app_ctx->rooms[room_id].giocatori[1];
            // Shift remaining players
            for (int k = 1; k < numero_giocatori - 1; k++) {
                app_ctx->rooms[room_id].giocatori[k] = app_ctx->rooms[room_id].giocatori[k + 1];
            }
            reset_player(app_ctx, room_id, numero_giocatori - 1);
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Host %d left room %d. New host is %d.\n", indice_utente, room_id, app_ctx->rooms[room_id].giocatori[0].indice_utente);
        } else {
             // This case should be covered by numero_giocatori - 1 == 0 check above
        }
    } else {
        // Non-host player is leaving
        if(app_ctx->rooms[room_id].giocatori[indice_giocatore].stepNumber > 0)
            update_steps_number(app_ctx, room_id, app_ctx->rooms[room_id].giocatori[indice_giocatore].stepNumber);
    
        for (int k = indice_giocatore; k < numero_giocatori -1; k++) {
            app_ctx->rooms[room_id].giocatori[k] = app_ctx->rooms[room_id].giocatori[k+1];
        }
        reset_player(app_ctx, room_id, numero_giocatori-1);
    }

    check_if_game_finished(app_ctx, room_id);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d left room %d\n", indice_utente, room_id);

    int next_user_candidate = dequeue(app_ctx, room_id);
    if (next_user_candidate != -1) {
        int slot = get_first_available_slot(app_ctx, room_id); // Should be available now
        if (slot != -1) {
            app_ctx->rooms[room_id].giocatori[slot].indice_utente = next_user_candidate;
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %d moved from queue to room %d\n", next_user_candidate, room_id);
        } else {
            log4c_category_log(log, LOG4C_PRIORITY_WARN, "No slot available for user %d from queue in room %d, though one was expected.\n", next_user_candidate, room_id);
            // This case should ideally not happen if logic is correct
        }
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Queue for room %d is empty, no user to move.\n", room_id);
    }
    
    pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
    return SUCCESS;
}

int start_game(AppContext* app_ctx, int indice_utente, int room_id) {
    log4c_category_t* log = initialize_log4c_category("start_game");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d trying to start game in room %d\n", indice_utente, room_id);

    pthread_mutex_lock(&app_ctx->rooms_mutex);
    if (room_id < 0 || room_id >= MAX_ROOMS || app_ctx->rooms[room_id].indice_stanza == -1) {
        pthread_mutex_unlock(&app_ctx->rooms_mutex);
        return ROOM_NOT_FOUND;
    }

    pthread_mutex_lock(&app_ctx->rooms[room_id].stanza_mutex);
    pthread_mutex_unlock(&app_ctx->rooms_mutex);

    if (app_ctx->rooms[room_id].giocatori[0].indice_utente != indice_utente) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return USER_NOT_HOST;
    }

    if (app_ctx->rooms[room_id].stato_gioco == GIOCO_IN_CORSO) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return GAME_ALREADY_STARTED;
    }

    if(calculate_players_number(app_ctx, room_id) < MIN_GIOCATORI) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return NOT_ENOUGH_PLAYERS;
    }

    app_ctx->rooms[room_id].stato_gioco = GIOCO_IN_CORSO;
    reset_messages(app_ctx, room_id); 
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Game started in room %d by user %d\n", room_id, indice_utente);
    
    pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
    return SUCCESS;
}

int calculate_step_number(AppContext* app_ctx, int room_id) {
    int stepNumber = 0;
    for (int j = 0; j < MAX_GIOCATORI; j++) {
        if(app_ctx->rooms[room_id].giocatori[j].indice_utente != -1 && strlen(app_ctx->rooms[room_id].giocatori[j].messaggio) > 0) {
            stepNumber++;
        }
    }
    return stepNumber;
}

int send_message(AppContext* app_ctx, int indice_utente, int room_id, const char *message) {
    log4c_category_t* log = initialize_log4c_category("send_message");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User %d trying to send message in room %d\n", indice_utente, room_id);

    pthread_mutex_lock(&app_ctx->rooms_mutex);
    if (room_id < 0 || room_id >= MAX_ROOMS || app_ctx->rooms[room_id].indice_stanza == -1) {
        pthread_mutex_unlock(&app_ctx->rooms_mutex);
        return ROOM_NOT_FOUND;
    }

    pthread_mutex_lock(&app_ctx->rooms[room_id].stanza_mutex);
    pthread_mutex_unlock(&app_ctx->rooms_mutex);

    int indice_giocatore = calculate_index_player(app_ctx, room_id, indice_utente);

    if (indice_giocatore == -1) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return USER_NOT_IN_ROOM;
    }

    if (app_ctx->rooms[room_id].stato_gioco != GIOCO_IN_CORSO) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return GAME_NOT_STARTED;
    }

    int next_player = calculate_next_player(app_ctx, room_id);
    if (next_player == -1 && app_ctx->rooms[room_id].stato_gioco != GIOCO_TERMINATO) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return NO_PLAYERS_AVAILABLE; // Should not happen if game is in progress and not finished
    }
    if (next_player != -1 && app_ctx->rooms[room_id].giocatori[next_player].indice_utente != indice_utente) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return NOT_PLAYER_TURN;
    }
     if (next_player == -1 && app_ctx->rooms[room_id].stato_gioco == GIOCO_TERMINATO) {
        // Game is finished, no more messages can be sent.
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return GAME_NOT_STARTED; // Or a more specific "GAME_FINISHED" status
    }

    if(strlen(app_ctx->rooms[room_id].giocatori[indice_giocatore].messaggio) > 0) {
        pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
        return USER_ALREADY_SENT_MESSAGE;
    }

    app_ctx->rooms[room_id].giocatori[indice_giocatore].stepNumber = calculate_step_number(app_ctx, room_id);
    strncpy(app_ctx->rooms[room_id].giocatori[indice_giocatore].messaggio, message, MAX_LUNGHEZZA_MESSAGGIO -1);
    app_ctx->rooms[room_id].giocatori[indice_giocatore].messaggio[MAX_LUNGHEZZA_MESSAGGIO -1] = '\0'; // Ensure null termination
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Message sent by user %d in room %d: %s\n", indice_utente, room_id, message);
    check_if_game_finished(app_ctx, room_id);
    
    pthread_mutex_unlock(&app_ctx->rooms[room_id].stanza_mutex);
    return SUCCESS;
}