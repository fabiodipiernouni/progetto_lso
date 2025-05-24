#include "app_context.h"
#include "../services/rooms/rooms.h" // For initialize_rooms, cleanup_all_room_mutexes
#include "../services/users/users.h" // For initialize_users
#include <stdlib.h> // For malloc, free
#include <pthread.h> // For pthread_mutex_init/destroy

AppContext *initialize_app_context() {
    AppContext *ctx = (AppContext *)malloc(sizeof(AppContext));
    if (!ctx) {
        return NULL;
    }

    initialize_rooms(ctx);
    pthread_mutex_init(&ctx->rooms_mutex, NULL);

    initialize_users(ctx);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        ctx->clients[i].socket = -1; // Initialize socket to -1 (not connected)
        ctx->clients[i].indice_client = i; // Assign client index
    }

    return ctx;
}

void destroy_app_context(AppContext *ctx) {
    if (!ctx)
        return;

    cleanup_all_room_mutexes(ctx);
    
    // Destroy the main rooms mutex
    pthread_mutex_destroy(&ctx->rooms_mutex);

    free(ctx);
}
