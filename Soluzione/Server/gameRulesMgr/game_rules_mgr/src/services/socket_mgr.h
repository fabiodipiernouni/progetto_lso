#ifndef GAME_RULES_MGR_SRC_SERVICES_SOCKET_MGR_H_
#define GAME_RULES_MGR_SRC_SERVICES_SOCKET_MGR_H_

#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>

// Forward declaration for AppContext to avoid circular dependencies
typedef struct AppContext AppContext;

typedef struct {
    int indice_client;
    int socket;
    struct sockaddr_in address;
} Client;

#define MAX_CLIENTS 30


int create_server_socket(AppContext* app_ctx, int *server_socket, int port);
int close_server_socket(AppContext* app_ctx, int * server_socket);

int create_client_socket(int *client_socket, char ip[], int port);

bool is_client_connected(Client *client);
void on_client_disconnect(AppContext* app_ctx, Client *client_ptr);

int socket_write(const int socket, const char *message);
int socket_read(const int socket, char response[], const int size);

#endif //GAME_RULES_MGR_SRC_SERVICES_SOCKET_MGR_H_
