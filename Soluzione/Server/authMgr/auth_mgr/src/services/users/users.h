#ifndef PRJ_SERVICE_USERS_H_
#define PRJ_SERVICE_USERS_H_

#include "../../entities/utente.h"

int registra_utente(utente *user);
int login(const char *email, const char *password, utente *user);
int ottieni_lingua_utente(const char *email, char *codice_lingua, char*descrizione_lingua);
int ottieni_utente(utente* user);
int aggiorna_lingua_utente(const char *email, const char *nuovo_codice_lingua);
int aggiorna_email_utente(const char *email, const char *nuova_email);
int aggiorna_password_utente(const char *email, const char *nuova_password);
int aggiorna_dati_utente(const char *email, const char *new_email, const char *new_password, const char *new_lang, const char *new_nome, const char *new_cognome);

#endif //PRJ_SERVICE_USERS_H_
