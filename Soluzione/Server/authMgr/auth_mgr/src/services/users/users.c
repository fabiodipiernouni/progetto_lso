#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <string.h>

#include <log4c.h>
#include "users.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "../../repository/database.h"

#include "../../repository/utente/utente.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../../repository/lingue/lingue.h"
#include "../../entities/utente.h"
#include "../security/security.h"

int registra_utente(utente* user) {

    char * hash_hex = NULL;
    int res;

    log4c_category_t* log = initialize_log4c_category("registra_utente");

    res = string_to_hash_hex(user->password, &hash_hex);

    if(res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to hash password\n");
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Password hashed\n");

        strcpy(user->password, hash_hex);

        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Registering user\n");

        res = ins_user(user);

        if (res != SUCCESS)
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to register user (insert res=%d)\n", res);
        else
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "User registered\n");
    }

    if(hash_hex != NULL) free(hash_hex);

    return res;
}

int login(const char *email, const char *password, utente *user) {
    int res = SUCCESS;
    char *hash_hex = NULL;

    log4c_category_t* log = initialize_log4c_category("login");

    int fetch_status;

    fetch_status = get_user(email, user);

    if (fetch_status == NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User not found\n");
        return NO_DATA_FOUND;
    }

    if (fetch_status != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to fetch data\n");
        return -2;
    }

    // Convert hash to hex string
    res = string_to_hash_hex(password, &hash_hex);

    // Compare the calculated hash with the stored hash
    if (strcmp(hash_hex, user->password) != 0) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Password mismatch\n");
        res = WRONG_PASSWORD;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "User logged in successfully\n");
        res = SUCCESS;
    }

    if(hash_hex != NULL) free(hash_hex);

    return res;
}

int ottieni_utente(utente*user) {
    log4c_category_t* log = initialize_log4c_category("ottieni_utente");

    int res = SUCCESS;

    if (strlen(user->email)==0)
        res = get_user_by_id(user->id_utente, user);
    else
        res = get_user(user->email, user);

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

int ottieni_lingua_utente(const char *email, char *codice_lingua, char*descrizione_lingua) {
    log4c_category_t* log = initialize_log4c_category("ottieni_lingua_utente");

    int res = get_user_lang(email, NULL, codice_lingua);

    if(res == NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "User not found\n");
        return -1;
    }

    if(res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to fetch data\n");
        return -2;
    }

    res = get_descrizione_lingua(codice_lingua, descrizione_lingua);

    if(res) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to get language\n");
        return -3;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Language retrieved\n");
    return SUCCESS;
}

int aggiorna_lingua_utente(const char *email, const char *new_language)
{
    log4c_category_t* log = initialize_log4c_category("aggiorna_lingua_utente");

    if(email == NULL || new_language == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "No parameters to update\n");
        return -1;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Updating utente %s.\n", email);
    int res = upd_user(email, NULL, NULL, new_language, NULL, NULL);
    if(res != SUCCESS && res != NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to update utente %s.\n", email);
        return -2;
    }

    return res;
}

int aggiorna_email_utente(const char *email, const char *new_email)
{
    log4c_category_t* log = initialize_log4c_category("aggiorna_email_utente");

    if(email == NULL || new_email == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "No parameters to update\n");
        return -1;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Updating utente %s.\n", email);
    int res = upd_user(email, new_email, NULL, NULL, NULL, NULL);
    if(res != 0 && res != NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to update utente %s.\n", email);
        return -2;
    }

    return res;
}

int aggiorna_password_utente(const char *email, const char *new_password)
{
    int res = SUCCESS;
    char *hash_hex = NULL;

    log4c_category_t* log = initialize_log4c_category("aggiorna_password_utente");

    if(email == NULL || new_password == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "No parameters to update\n");
        return -1;
    }

    // Convert hash to hex string
    res = string_to_hash_hex(new_password, &hash_hex);

    if(res == SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Updating utente %s.\n", email);
        res = upd_user(email, NULL, hash_hex, NULL, NULL, NULL);
        if (res != 0 && res != NO_DATA_FOUND) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to update utente %s.\n", email);
        }
    }

    if(hash_hex != NULL) free(hash_hex);

    return res;
}

int aggiorna_dati_utente(const char *email, const char *new_email, const char *new_password, const char *new_lang, const char *new_nome, const char *new_cognome)
{
    int res = SUCCESS;
    char *hash_hex = NULL;
    const char *password_to_update = NULL;

    log4c_category_t* log = initialize_log4c_category("aggiorna_dati_utente");

    if(email == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Email is required to update user data\n");
        return -1;
    }

    if(new_email == NULL && new_password == NULL && new_lang == NULL && new_nome == NULL && new_cognome == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "No data provided to update for user %s\n", email);
        return SUCCESS; // No update needed
    }

    // Hash the password if it's provided
    if (new_password != NULL && strlen(new_password) > 0) {
        res = string_to_hash_hex(new_password, &hash_hex);
        if (res != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to hash new password for user %s\n", email);
            return res; // Propagate hashing error
        }
        password_to_update = hash_hex;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Updating user %s.\n", email);

    // Call the repository function with all provided data
    res = upd_user(email, new_email, password_to_update, new_lang, new_nome, new_cognome);

    if (res != SUCCESS && res != NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to update user %s (repo res=%d).\n", email, res);
    } else if (res == NO_DATA_FOUND) {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "User %s not found for update.\n", email);
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "User %s updated successfully.\n", email);
    }

    // Free the hashed password if it was allocated
    if (hash_hex != NULL) {
        free(hash_hex);
    }

    return res;
}
