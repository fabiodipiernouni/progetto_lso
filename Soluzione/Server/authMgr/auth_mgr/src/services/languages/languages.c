#include "languages.h"
#include "../../repository/lingue/lingue.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"

#include <log4c.h>

int ottieni_lingua(const char *codice_lingua, char* descrizione_lingua) {
    log4c_category_t* log = initialize_log4c_category("ottieni_utente");

    int res = get_descrizione_lingua(codice_lingua, descrizione_lingua);

    if(res == NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User not found\n");
    }
    else if(res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to fetch data\n");
    }
    else
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "User retrieved\n");

    return res;
}

int ottieni_lingue(lingua** lingue, int* num_lingue) {
    log4c_category_t* log = initialize_log4c_category("ottieni_lingue");

    int res = get_lingue(lingue, num_lingue);

    if(res == NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Languages not found\n");
    }
    else if(res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to fetch data\n");
    }
    else
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Languages retrieved\n");

    return res;
}