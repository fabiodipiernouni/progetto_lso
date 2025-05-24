#ifndef GAME_RULES_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_
#define GAME_RULES_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_

#include "../socket_mgr.h"
typedef struct AppContext AppContext;

#include "../../entities/utente.h"
#include "../../entities/stanza.h"

int accept_new_client(AppContext* app_ctx, int server_socket);
int handle_client_message(AppContext* app_ctx, Client *client, char * buffer, int bytes_read);
int handle_client_disconnect(AppContext* app_ctx, int indice_client, char *error_msg);

#endif //GAME_RULES_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_
