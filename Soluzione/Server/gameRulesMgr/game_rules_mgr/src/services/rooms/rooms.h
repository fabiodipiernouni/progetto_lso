#ifndef PRJ_SERVICE_ROOMS_H_
#define PRJ_SERVICE_ROOMS_H_

#include "../../entities/stanza.h"
#include "../../entities/utente.h"
#include "../../utils/app_context.h"


void initialize_rooms(AppContext* app_ctx);
int create_room(AppContext* app_ctx, int indice_utente_host, char *room_name, ordine_gioco_t ordine_gioco, int *room_id);
int join_room(AppContext* app_ctx, int indice_utente, int room_id);
int leave_room(AppContext* app_ctx, int indice_utente, int room_id);
int start_game(AppContext* app_ctx, int indice_utente, int room_id);
int send_message(AppContext* app_ctx, int indice_utente, int room_id, const char *message);
void check_if_game_finished(AppContext* app_ctx, int room_id);
int calculate_players_number(AppContext* app_ctx, const int room_id);
int calculate_next_player(AppContext* app_ctx, int room_id);
void cleanup_all_room_mutexes(AppContext* app_ctx);

#endif //PRJ_SERVICE_ROOMS_H_
