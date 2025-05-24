#ifndef PRJ_SERVICE_USERS_H_
#define PRJ_SERVICE_USERS_H_

#include "../../entities/utente.h"
#include "../socket_mgr.h"
#include "../../utils/app_context.h"

void initialize_users(AppContext* app_ctx);
int check_same_user_id_already_set(AppContext* app_ctx, const int id_utente, const int indice_client);
bool has_user_set_id(AppContext* app_ctx, const int indice_client);
int get_room_by_user_index(AppContext* app_ctx, const int indice_utente);
int disconnect_user(AppContext* app_ctx, const int indice_utente);

#endif //PRJ_SERVICE_USERS_H_
