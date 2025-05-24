#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

#include "mapping.h"

void get_invalid_room_name_msg(int res, char cause[]) {
    switch (res) {
        case EMPTY_FIELD:
            strcpy(cause, "Room name is empty");
        break;
        case INVALID_FIELD_LENGTH:
            strcpy(cause, "Room name is too long");
        break;
        case INVALID_FIELD_TYPE:
            strcpy(cause, "Room name has invalid characters");
        break;
        case ROOM_NAME_TAKEN:
            strcpy(cause, "Room name already taken");
        break;
        default:
            strcpy(cause, "Invalid room name");
        break;
    }
}