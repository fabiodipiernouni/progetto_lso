#ifndef AUTH_MGR_SRC_ENTITIES_UTENTE_H_
#define AUTH_MGR_SRC_ENTITIES_UTENTE_H_

typedef struct _UTENTE_ {

    long id_utente;
    char nome[128];
    char cognome[128];
    char email[128];
    char password[512];
    char codice_lingua[10];
} utente;

#endif //AUTH_MGR_SRC_ENTITIES_UTENTE_H_
