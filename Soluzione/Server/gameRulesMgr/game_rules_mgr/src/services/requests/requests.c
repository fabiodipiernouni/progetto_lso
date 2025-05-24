#include "requests.h"
#include <stdio.h>
#include "../../utils/mapping.h"

int create_refresh_room_request(int indice_stanza, char request[]) {
    return sprintf(request, "refresh        {\"id_stanza\":%d}", indice_stanza) ? SUCCESS : FAILURE;
}