#ifndef GAME_RULES_MGR_SRC_UTILS_APP_CONTEXT_H_
#define GAME_RULES_MGR_SRC_UTILS_APP_CONTEXT_H_

#include <pthread.h>
#include "../entities/stanza.h"    // Provides Stanza, MAX_ROOMS
#include "../entities/utente.h"    // Provides Utente
#include "../services/socket_mgr.h" // Provides Client, MAX_CLIENTS

// AppContext struct definition
typedef struct AppContext {
    Client clients[MAX_CLIENTS];
    Stanza rooms[MAX_ROOMS];
    pthread_mutex_t rooms_mutex;
    Utente users[MAX_CLIENTS]; 
} AppContext;

// Function declarations
AppContext* initialize_app_context();
void destroy_app_context(AppContext* context);

#endif // GAME_RULES_MGR_SRC_UTILS_APP_CONTEXT_H_
