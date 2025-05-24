#include <stdlib.h>
#include <oci.h>
#include <log4c.h>
#include <stdio.h>
#include "lingue.h"
#include <string.h>
#include "../database.h"
#include "../../utils/helper.h"
#include "../utils/helper.h"
#include "../../entities/lingua.h"
#include "../../utils/mapping.h"

int get_lingue(lingua** lingue, int* num_lingue)
{
    OCIStmt *stmt;
    OCIDefine *def1, *def2;
    char *sql = "SELECT codice, descrizione FROM LINGUE";
    *num_lingue = 0;
    *lingue = NULL;

    log4c_category_t* log = initialize_log4c_category("get_lingue");

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

    sword res_status;

    do {

        char codice_lingua[10] = {0};
        char descrizione_lingua[128] = {0};

        if (OCIDefineByPos(stmt, &def1, errhp, 1, (void *)codice_lingua, 10, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
            OCIDefineByPos(stmt, &def2, errhp, 2, (void *)descrizione_lingua, 128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
            log_oci_error("Failed to define variables", log, errhp);
            OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -3;
        }

        if (OCIStmtExecute(svchp, stmt, errhp, 0, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
            log_oci_error("Failed to execute statement", log, errhp);
            OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return -4;
        }

        res_status = OCIStmtFetch2(stmt, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT);

        if(res_status == OCI_SUCCESS) {
            if (*num_lingue == 0) {
                *lingue = (lingua*)malloc(sizeof(lingua) * 10);
                if (*lingue == NULL) {
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate memory for languages\n");
                    OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
                    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
                    return -5;
                }
            } else if (*num_lingue % 10 == 0) {
                *lingue = (lingua*)realloc(*lingue, sizeof(lingua) * (*num_lingue + 10));
                if (*lingue == NULL) {
                    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to reallocate memory for languages\n");
                    OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
                    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
                    return -6;
                }
            }

            // Copy the data into the lingua structure
            strcpy((*lingue)[*num_lingue].codice_lingua, codice_lingua);
            strcpy((*lingue)[*num_lingue].descrizione_lingua, descrizione_lingua);

            (*num_lingue)++;
        }
    } while (res_status == OCI_SUCCESS);

    int res = SUCCESS;
    if(res_status != OCI_NO_DATA) {
        log_oci_error("Failed to fetch data", log, errhp);
        res = FAILURE;
    }
    else if(res_status == OCI_NO_DATA && *num_lingue == 0) {
        log4c_category_log(log, LOG4C_PRIORITY_WARN, "No languages found, maybe the table must be filled.\n");
        res = NO_DATA_FOUND;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "%d languages retrieved\n", *num_lingue);
    }

    OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    return res;
}

int get_descrizione_lingua(const char *codice_lingua, char *descrizione_lingua)
{
    OCIStmt *stmt;
    OCIBind *bnd1 = NULL;
    OCIDefine *def;
    char *sql = "SELECT descrizione FROM LINGUE WHERE codice = :1";

    log4c_category_t* log = initialize_log4c_category("get_descrizione_lingua");

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
            OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
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

    if (OCIBindByPos(stmt, &bnd1, errhp, 1, (void *)codice_lingua, (sb4)strlen(codice_lingua) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to bind variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -4;
    }

    if (OCIDefineByPos(stmt, &def, errhp, 1, (void *)descrizione_lingua, (sb4)128, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to define variables", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -5;
    }

    if (OCIStmtExecute(svchp, stmt, errhp, 1, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS)
    {
        log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
        return -6;
    }

    OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
    return SUCCESS;
}