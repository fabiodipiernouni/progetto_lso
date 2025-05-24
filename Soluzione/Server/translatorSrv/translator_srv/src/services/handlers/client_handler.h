#ifndef AUTHMGR_AUTH_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_
#define AUTHMGR_AUTH_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_

int handle_client_message(int client_socket, char * buffer, int bytes_read);
int accept_new_client(int server_socket);

#endif //AUTHMGR_AUTH_MGR_SRC_SERVICES_HANDLERS_CLIENT_HANDLER_H_
