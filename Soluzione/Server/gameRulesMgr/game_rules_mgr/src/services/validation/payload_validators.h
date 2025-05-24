#ifndef GAME_RULES_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_
#define GAME_RULES_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_

typedef struct AppContext AppContext;

#include "../../entities/utente.h"
#include "../../entities/stanza.h"

int validate_create_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, char *room_name, ordine_gioco_t *ordine_gioco);
int validate_join_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza);
int validate_leave_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza);
int validate_get_room_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza);
int validate_start_game_payload(AppContext* app_ctx, const char *payload, char*error_msg, int* indice_stanza);
int validate_send_message_payload(const char *payload, char*error_msg, char *message);
int validate_set_user_payload(const char *payload, char*error_msg, char* email);

#endif //GAME_RULES_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_
