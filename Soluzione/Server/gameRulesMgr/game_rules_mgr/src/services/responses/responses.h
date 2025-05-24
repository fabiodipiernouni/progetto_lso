#ifndef GAME_RULES_MGR_SRC_SERVICES_REPONSES_RESPONSES_H_
#define GAME_RULES_MGR_SRC_SERVICES_REPONSES_RESPONSES_H_

#include "../../entities/utente.h"
#include "../../entities/stanza.h"
#include "../../utils/app_context.h"

#define STATUS_OK "OK"
#define STATUS_ERROR "ERROR"
#define STATUS_USER_ALREADY_SET "USER_ALREADY_SET"
#define STATUS_USER_NOT_SET "USER_NOT_SET"
#define STATUS_ROOM_LIMIT_REACHED "ROOM_LIMIT_REACHED"
#define STATUS_ROOM_NAME_TAKEN "ROOM_NAME_TAKEN"
#define STATUS_ROOM_NOT_FOUND "ROOM_NOT_FOUND"
#define STATUS_QUEUE_FULL "QUEUE_FULL"
#define STATUS_USER_ALREADY_IN_ROOM "USER_ALREADY_IN_ROOM"
#define STATUS_USER_NOT_IN_ROOM "USER_NOT_IN_ROOM"
#define STATUS_USER_NOT_HOST "USER_NOT_HOST"
#define STATUS_GAME_ALREADY_STARTED "GAME_ALREADY_STARTED"
#define STATUS_NOT_ENOUGH_PLAYERS "NOT_ENOUGH_PLAYERS"
#define STATUS_NO_PLAYERS_AVAILABLE "NO_PLAYERS_AVAILABLE"
#define STATUS_NOT_PLAYER_TURN "NOT_PLAYER_TURN"
#define STATUS_GAME_NOT_STARTED "GAME_NOT_STARTED"
#define STATUS_USER_ALREADY_SENT_MESSAGE "USER_ALREADY_SENT_MESSAGE"
#define STATUS_USER_SET_BY_ANOTHER_CLIENT "USER_SET_BY_ANOTHER_CLIENT"


char* create_simple_response(const char* esito, const char* message);
char* create_create_room_response(AppContext* app_ctx, int indice_stanza);
char* create_get_rooms_response(AppContext* app_ctx);
char* create_get_room_response(AppContext* app_ctx, int indice_stanza);
char* create_get_user_response(AppContext* app_ctx, int id_utente);
void free_response(char** response);

#endif //GAME_RULES_MGR_SRC_SERVICES_REPONSES_RESPONSES_H_
