#include "utente.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <log4c.h>
#include <oci.h>

#include "../database.h"
#include "../../utils/mapping.h"
#include "../../utils/helper.h"
#include "../utils/helper.h"
#include "../../entities/utente.h"

int ins_user(utente* user)
{
    char *sql = "INSERT INTO UTENTE (id_utente, nome, cognome, email, password, codice_lingua) VALUES (utente_seq.nextval, :1, :2, :3, :4, :5) RETURNING id_utente INTO :6";
    OCIStmt *stmt;
    OCIBind *bnd1 = NULL, *bnd2 = NULL, *bnd3 = NULL, *bnd4 = NULL, *bnd5 = NULL, *bnd6 = NULL;

    log4c_category_t* log = initialize_log4c_category("ins_user");

    // allocate the statement handle
    if (OCIHandleAlloc(envhp, (void **)&stmt, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS)
    {
        log_oci_error("Failed to allocate statement handle", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -1;
    }

    // Allocate a new error handle if needed
    if (errhp == NULL) {
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
            return -1;
        }
    }

    if (OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to prepare statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -2;
    }

    if (OCIBindByPos(stmt, &bnd1, errhp, 1, (void *)user->nome, (sb4)strlen(user->nome) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmt, &bnd2, errhp, 2, (void *)user->cognome, (sb4)strlen(user->cognome) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmt, &bnd3, errhp, 3, (void *)user->email, (sb4)strlen(user->email) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmt, &bnd4, errhp, 4, (void *)user->password, (sb4)strlen(user->password) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmt, &bnd5, errhp, 5, (void *)user->codice_lingua, (sb4)strlen(user->codice_lingua) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmt, &bnd6, errhp, 6, (void *)&(user->id_utente), sizeof(user->id_utente), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to bind variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -3;
    }

    if (OCIStmtExecute(svchp, stmt, errhp, 1, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        int error_code = log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        if(error_code == 1) {
            return EMAIL_ALREADY_USED;
        }
        return -4;
    }

    if (OCITransCommit(svchp, errhp, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to commit transaction", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -5;
    }

    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    log4c_category_log(log, LOG4C_PRIORITY_INFO, "User inserted\n");

    return 0;
}

int get_user(const char *email, utente*user)
{
    OCIStmt *stmt;
    OCIBind *bnd1 = NULL;
    OCIDefine *def1, *def2, *def3, *def4, *def5;
    char *sql = "SELECT id_utente, nome, cognome, password, codice_lingua FROM UTENTE WHERE email = :1";

    log4c_category_t* log = initialize_log4c_category("get_user");

    // allocate the statement handle
    if (OCIHandleAlloc(envhp, (void **)&stmt, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS)
    {
        log_oci_error("Failed to allocate statement handle", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -1;
    }

    // Allocate a new error handle if needed
    if (errhp == NULL) {
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
            return -2;
        }
    }

    if (OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to prepare statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -3;
    }

    if (OCIBindByPos(stmt, &bnd1, errhp, 1, (void *)email, (sb4)strlen(email) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to bind variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -4;
    }

    if (OCIDefineByPos(stmt, &def1, errhp, 1, (void *)&(user->id_utente), sizeof(user->id_utente), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def2, errhp, 2, (void *)user->nome, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def3, errhp, 3, (void *)user->cognome, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def4, errhp, 4, (void *)user->password, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def5, errhp, 5, (void *)user->codice_lingua, 10, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to define variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -5;
    }

    if (OCIStmtExecute(svchp, stmt, errhp, 0, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -6;
    }

    sword fetch_status = OCIStmtFetch2(stmt, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT);

    if(fetch_status == OCI_ERROR) {
        log_oci_error("Failed to fetch data", log, errhp);
    }
    else {
        log_oci_status(log, fetch_status);
        strcpy(user->email, email);
    }

    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    return fetch_status == OCI_NO_DATA ? NO_DATA_FOUND : (fetch_status == OCI_SUCCESS?SUCCESS:FAILURE);
}

int get_user_by_id(const long id_utente, utente*user)
{
    OCIStmt *stmt;
    OCIBind *bnd1 = NULL;
    OCIDefine *def1, *def2, *def3, *def4, *def5;
    char *sql = "SELECT email, nome, cognome, password, codice_lingua FROM UTENTE WHERE id_utente = :1";

    log4c_category_t* log = initialize_log4c_category("get_user_by_id");

    // allocate the statement handle
    if (OCIHandleAlloc(envhp, (void **)&stmt, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS)
    {
        log_oci_error("Failed to allocate statement handle", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -1;
    }

    // Allocate a new error handle if needed
    if (errhp == NULL) {
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
            return -2;
        }
    }

    if (OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to prepare statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -3;
    }

    if (OCIBindByPos(stmt, &bnd1, errhp, 1, (void *)&id_utente, sizeof(id_utente), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to bind variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -4;
    }

    if (OCIDefineByPos(stmt, &def1, errhp, 1, (void *)user->email, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def2, errhp, 2, (void *)user->nome, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def3, errhp, 3, (void *)user->cognome, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def4, errhp, 4, (void *)user->password, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmt, &def5, errhp, 5, (void *)user->codice_lingua, 10, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to define variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -5;
    }

    if (OCIStmtExecute(svchp, stmt, errhp, 0, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -6;
    }

    sword fetch_status = OCIStmtFetch2(stmt, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT);

    if(fetch_status == OCI_ERROR) {
        log_oci_error("Failed to fetch data", log, errhp);
    }
    else {
        log_oci_status(log, fetch_status);
        user->id_utente = id_utente;
    }

    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    return fetch_status == OCI_NO_DATA ? NO_DATA_FOUND : (fetch_status == OCI_SUCCESS?SUCCESS:FAILURE);
}

int get_user_lang(const char *email, const long *id_user, char *cod_lang)
{
    OCIStmt *stmt;
    OCIBind *bnd1 = NULL;
    OCIDefine *def1;
    char *sql = id_user != NULL? "SELECT codice_lingua FROM UTENTE WHERE id_utente = :1": "SELECT codice_lingua FROM UTENTE WHERE email = :1";

    log4c_category_t* log = initialize_log4c_category("get_user_lang");

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Getting user language via sql: %s and field [%s].\n", sql, email);

    // allocate the statement handle
    if (OCIHandleAlloc(envhp, (void **)&stmt, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS)
    {
        log_oci_error("Failed to allocate statement handle", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -1;
    }

    // Allocate a new error handle if needed
    if (errhp == NULL) {
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
            return -1;
        }
    }

    if (OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to prepare statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -2;
    }

    sword bind_res = id_user != NULL ?
        OCIBindByPos(stmt, &bnd1, errhp, 1, (void *)id_user, sizeof(*id_user), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT):
        OCIBindByPos(stmt, &bnd1, errhp, 1, (void *)email, (sb4)strlen(email) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);

    if (bind_res != OCI_SUCCESS)
    {
        log_oci_error("Failed to bind variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -3;
    }

    if (OCIDefineByPos(stmt, &def1, errhp, 1, (void *)cod_lang, 10, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to define variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -4;
    }

    if (OCIStmtExecute(svchp, stmt, errhp, 0, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -5;
    }

    sword fetch_status = OCIStmtFetch2(stmt, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT);

    if(fetch_status == OCI_ERROR) {
        log_oci_error("Failed to fetch data", log, errhp);
    }
    else
        log_oci_status(log, fetch_status);

    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    return fetch_status == OCI_NO_DATA ? NO_DATA_FOUND : (fetch_status == OCI_SUCCESS?SUCCESS:FAILURE);
}

int upd_user(const char *email, const char *new_email, const char* new_password, const char *new_cod_lang, const char *new_nome, const char *new_cognome)
{
    OCIStmt *stmt;
    OCIBind *binds[6] = {NULL, NULL, NULL, NULL, NULL, NULL}; // Array for binding handles (email, password, lang, nome, cognome, WHERE email)
    sword status;

    char sql[1024] = "UPDATE UTENTE SET";
    bool first = true;
    int bind_pos = 1;

    log4c_category_t* log = initialize_log4c_category("upd_user");

    if(new_email == NULL && new_password == NULL && new_cod_lang == NULL && new_nome == NULL && new_cognome == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "No parameters to update\n");
        return -1;
    }

    // Build the SQL query with placeholders
    if(new_email != NULL) {
        strcat(sql, " email = :1");
        first = false;
        bind_pos++;
    }

    if(new_password != NULL) {
        if(!first) {
            strcat(sql, ",");
        }
        char spr[256];
        sprintf(spr, " password = :%d", bind_pos);
        strcat(sql, spr);
        first = false;
        bind_pos++;
    }

    if(new_cod_lang != NULL) {
        if(!first) {
            strcat(sql, ",");
        }
        char spr[256];
        sprintf(spr, " codice_lingua = :%d", bind_pos);
        strcat(sql, spr);
        first = false;
        bind_pos++;
    }

    if(new_nome != NULL) {
        if(!first) {
            strcat(sql, ",");
        }
        char spr[256];
        sprintf(spr, " nome = :%d", bind_pos);
        strcat(sql, spr);
        first = false;
        bind_pos++;
    }

    if(new_cognome != NULL) {
        if(!first) {
            strcat(sql, ",");
        }
        char spr[256];
        sprintf(spr, " cognome = :%d", bind_pos);
        strcat(sql, spr);
        bind_pos++;
    }

    char spr[256];
    sprintf(spr, " WHERE email = :%d", bind_pos);
    strcat(sql, spr);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Updating user with sql: %s\n", sql);

    // Allocate a new error handle if needed
    if (errhp == NULL) {
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
            return -1;
        }
    }

    // Allocate the statement handle
    if (OCIHandleAlloc(envhp, (void **)&stmt, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS)
    {
        log_oci_error("Failed to allocate statement handle", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        return -2;
    }

    if (OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to prepare statement", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -3;
    }

    // Bind the parameters dynamically
    bind_pos = 1; // Reset bind position for binding
    if(new_email != NULL) {
        status = OCIBindByPos(stmt, &binds[0], errhp, bind_pos++,
                              (void *)new_email, (sb4)strlen(new_email) + 1,
                              SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) {
            log_oci_error("Failed to bind new_email", log, errhp);
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -3;
        }
    }

    if(new_password != NULL) {
        status = OCIBindByPos(stmt, &binds[1], errhp, bind_pos++,
                              (void *)new_password, (sb4)strlen(new_password) + 1,
                              SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) {
            log_oci_error("Failed to bind new_password", log, errhp);
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -3;
        }
    }

    if(new_cod_lang != NULL) {
        status = OCIBindByPos(stmt, &binds[2], errhp, bind_pos++,
                              (void *)new_cod_lang, (sb4)strlen(new_cod_lang) + 1,
                              SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) {
            log_oci_error("Failed to bind new_cod_lang", log, errhp);
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -3;
        }
    }

    if(new_nome != NULL) {
        status = OCIBindByPos(stmt, &binds[3], errhp, bind_pos++,
                              (void *)new_nome, (sb4)strlen(new_nome) + 1,
                              SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) {
            log_oci_error("Failed to bind new_nome", log, errhp);
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -3;
        }
    }

    if(new_cognome != NULL) {
        status = OCIBindByPos(stmt, &binds[4], errhp, bind_pos++,
                              (void *)new_cognome, (sb4)strlen(new_cognome) + 1,
                              SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) {
            log_oci_error("Failed to bind new_cognome", log, errhp);
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -3;
        }
    }


    // Bind the WHERE clause parameter (email)
    status = OCIBindByPos(stmt, &binds[5], errhp, bind_pos,
                          (void *)email, (sb4)strlen(email) + 1,
                          SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    if (status != OCI_SUCCESS) {
        log_oci_error("Failed to bind email", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -3;
    }

    if (OCIStmtExecute(svchp, stmt, errhp, 1, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -4;
    }

    // Check if any rows were updated
    ub4 row_count = 0;
    OCIAttrGet(stmt, OCI_HTYPE_STMT, &row_count, NULL, OCI_ATTR_ROW_COUNT, errhp);

    int res = 0;
    if (row_count == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "No rows were updated - email '%s' might not exist\n", email);
        res = NO_DATA_FOUND;
    } else {
        log4c_category_log(log, LOG4C_PRIORITY_INFO, "Updated %d rows\n", row_count);
        res = SUCCESS;
    }

    if (OCITransCommit(svchp, errhp, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to commit transaction", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -5;
    }

    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    log4c_category_log(log, LOG4C_PRIORITY_INFO, "User updated\n");
    return res;
}
