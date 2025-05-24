#ifndef GAME_RULES_MGR_SRC_ENTITIES_UTENTE_H_
#define GAME_RULES_MGR_SRC_ENTITIES_UTENTE_H_

#define MAX_NOME_UTENTE 128
#define MAX_COGNOME_UTENTE 128
#define MAX_EMAIL_UTENTE 128
#define MAX_CODICE_LINGUA_UTENTE 10

typedef struct {
    int indice_utente;
    int id_utente;
    char nome[MAX_NOME_UTENTE];
    char cognome[MAX_COGNOME_UTENTE];
    char email[MAX_EMAIL_UTENTE];
    char codice_lingua[MAX_CODICE_LINGUA_UTENTE];
} Utente;

#endif //GAME_RULES_MGR_SRC_ENTITIES_UTENTE_H_
