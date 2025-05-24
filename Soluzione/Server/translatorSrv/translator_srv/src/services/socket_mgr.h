#ifndef AUTH_MGR_SRC_SERVICES_SOCKET_MGR_H_
#define AUTH_MGR_SRC_SERVICES_SOCKET_MGR_H_

#include <netinet/in.h>
#include <stdbool.h>

typedef struct {
    int socket;
    struct sockaddr_in address;
} Client;

#define MAX_CLIENTS 30

int create_server_socket(int *server_socket, struct sockaddr_in*server_addr, int port);
int close_server_socket(int * server_socket, Client*clients);

bool is_client_connected(Client *client);

#endif //AUTH_MGR_SRC_SERVICES_SOCKET_MGR_H_
