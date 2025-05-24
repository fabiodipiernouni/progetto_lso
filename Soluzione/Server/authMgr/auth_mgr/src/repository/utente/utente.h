#ifndef AUTHMGR_AUTH_MGR_SRC_REPOSITORY_UTENTE_UTENTE_H_
#define AUTHMGR_AUTH_MGR_SRC_REPOSITORY_UTENTE_UTENTE_H_

#include <log4c.h>
#include <oci.h>

#include "../../entities/utente.h"

int ins_user(utente *);
int get_user(const char *, utente*);
int get_user_by_id(const long, utente *);
int get_user_lang(const char *, const long *, char *);
int upd_user(const char *email, const char *new_email, const char* new_password, const char *new_cod_lang, const char *new_nome, const char *new_cognome);

#endif //AUTHMGR_AUTH_MGR_SRC_REPOSITORY_UTENTE_UTENTE_H_
