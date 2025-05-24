#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

#include "mapping.h"

void get_invalid_password_msg(int res, char *cause) {
    switch (res) {
        case PASSWORD_TOO_SHORT:
            strcpy(cause, "Password too short");
            break;
        case PASSWORD_NO_LOWERCASE:
            strcpy(cause, "Password has no lowercase letter");
            break;
        case PASSWORD_NO_UPPERCASE:
            strcpy(cause, "Password has no uppercase letter");
            break;
        case PASSWORD_NO_DIGIT:
            strcpy(cause, "Password has no digit");
            break;
        default:
            strcpy(cause, "Invalid password");
            break;
    }
}