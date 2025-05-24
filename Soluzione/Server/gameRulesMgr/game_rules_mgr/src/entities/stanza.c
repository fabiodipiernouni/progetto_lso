#include "stanza.h"
#include <string.h>

void get_ordine_gioco_string(ordine_gioco_t ordine_gioco, char res[]) {
    switch(ordine_gioco) {
        case ORARIO:
            strcpy(res, "ORARIO");
            break;
        case ANTIORARIO:
            strcpy(res, "ANTIORARIO");
    }
}

void get_stato_gioco_string(stato_gioco_t stato_gioco, char res[]) {
    switch(stato_gioco) {
        case IN_ATTESA_DI_GIOCATORI:
            strcpy(res, "IN_ATTESA_DI_GIOCATORI");
            break;
        case GIOCO_IN_CORSO:
            strcpy(res, "GIOCO_IN_CORSO");
            break;
        case GIOCO_TERMINATO:
            strcpy(res, "GIOCO_TERMINATO");
    }
}
