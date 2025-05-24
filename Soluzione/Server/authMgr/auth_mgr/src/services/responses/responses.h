#ifndef AUTH_MGR_SRC_SERVICES_REPONSES_RESPONSES_H_
#define AUTH_MGR_SRC_SERVICES_REPONSES_RESPONSES_H_

#include "../../entities/utente.h"
#include "../../entities/lingua.h"

#define STATUS_OK "OK"
#define STATUS_ERROR "ERROR"
#define STATUS_LOGIN_WRONG_PASSWORD "WRONG_PASSWORD"
#define STATUS_NO_DATA_FOUND "NO_DATA_FOUND"
#define STATUS_EMAIL_ALREADY_USED "EMAIL_ALREADY_USED"

char* create_simple_response(const char* esito, const char* message);
char* create_register_user_response(int id_utente);
char* create_get_user_response(const utente* user);
char* create_get_language_response(const lingua* lang);
char* create_get_languages_response(const lingua* langs, const int n_langs);
void free_response(char** response);

#endif //AUTH_MGR_SRC_SERVICES_REPONSES_RESPONSES_H_
