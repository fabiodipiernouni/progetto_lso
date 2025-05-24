#ifndef AUTH_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_
#define AUTH_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_

#include "../../entities/utente.h"
#include "../../entities/lingua.h"

int validate_update_user_payload(const char *payload, char*error_msg, char *email, char *new_email, char *new_password, char *new_lang, char *new_nome, char *new_cognome);
int validate_get_user_payload(const char *payload, char*error_msg, char* email, long* user_id);
int validate_register_user_payload(const char *payload, char*error_msg, utente* user);
int validate_login_payload(const char *payload, int*id_session, char *email, char *password, char*error_msg);
int validate_get_language_payload(const char *payload, char*error_msg, lingua* lang);

#endif //AUTH_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_
