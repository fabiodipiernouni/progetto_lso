#ifndef AUTHMGR_AUTH_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_
#define AUTHMGR_AUTH_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_

#include "../socket_mgr.h"

int handle_client_message(Client *client, char * buffer, int bytes_read);

#endif //AUTHMGR_AUTH_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_
