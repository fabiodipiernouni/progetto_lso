#ifndef GAME_RULES_MGR_SRC_ENTITIES_STANZA_H_
#define GAME_RULES_MGR_SRC_ENTITIES_STANZA_H_

#define MAX_ROOMS 50

#define MAX_GIOCATORI 10
#define MAX_CODA 20
#define MIN_GIOCATORI 4
#define MAX_NOME_STANZA 128
#define MAX_LUNGHEZZA_MESSAGGIO 51

#include <pthread.h>

typedef enum {
    ORARIO,
    ANTIORARIO
} ordine_gioco_t;

typedef enum {
    IN_ATTESA_DI_GIOCATORI,
    GIOCO_IN_CORSO,
    GIOCO_TERMINATO
} stato_gioco_t;

typedef struct {
    int indice_utente;
    char messaggio[MAX_LUNGHEZZA_MESSAGGIO];
    int stepNumber;
} Giocatore;

typedef struct {
    int indice_stanza;
    char nome[MAX_NOME_STANZA];
    pthread_mutex_t stanza_mutex; // Specific mutex per room
    ordine_gioco_t ordine_gioco;
    stato_gioco_t stato_gioco;
    Giocatore giocatori[MAX_GIOCATORI];  // host is always in position 0
    int coda[MAX_CODA]; // queue of players waiting to join
} Stanza;

void get_ordine_gioco_string(ordine_gioco_t ordine_gioco, char res[]);
void get_stato_gioco_string(stato_gioco_t stato_gioco, char res[]);


#endif //GAME_RULES_MGR_SRC_ENTITIES_STANZA_H_
