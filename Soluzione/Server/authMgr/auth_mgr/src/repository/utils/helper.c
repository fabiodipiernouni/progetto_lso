#include "helper.h"
#include "../../utils/helper.h"
#include "../../utils/mapping.h"
#include "../database.h"

#include <log4c.h>
#include <oci.h>
#include <string.h>

int log_oci_error(const char* message, log4c_category_t*log, OCIError *errhp) {
    text errbuf[512];
    int errcode = 0;
    OCIErrorGet((void *)errhp, (ub4)1, (text *)NULL, (sb4*)&errcode, errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
    log4c_category_log(log, LOG4C_PRIORITY_ERROR, "%s: %s\n", message, errbuf);
    return errcode;
}

// Error handling function
void log_oci_status(log4c_category_t* log, sword status) {

    switch (status) {
        case OCI_SUCCESS:
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI Status is OCI_SUCCESS\n");
            break;
        case OCI_SUCCESS_WITH_INFO:
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI Status is OCI_SUCCESS_WITH_INFO\n");
            break;
        case OCI_NEED_DATA:
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI Status is OCI_NEED_DATA\n");
            break;
        case OCI_NO_DATA:
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI Status is OCI_NO_DATA\n");
            break;
        case OCI_ERROR:
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "OCI Status is OCI_ERROR\n");
            break;
        case OCI_INVALID_HANDLE:
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "OCI Status is OCI_INVALID_HANDLE\n");
            break;
        case OCI_STILL_EXECUTING:
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI Status is OCI_STILL_EXECUTING\n");
            break;
        case OCI_CONTINUE:
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI Status is OCI_CONTINUE\n");
            break;
        default:
            break;
    }
}

// Helper function to test if DB connection is alive
int execute_db_ping() {
    sword status;
    OCIStmt *stmt = NULL;

    log4c_category_t* log = initialize_log4c_category("execute_db_ping");

    if (envhp == NULL || errhp == NULL || svchp == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Database connection not initialized\n");
        return FAILURE;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Allocating statement handle\n");
    status = OCIHandleAlloc(envhp, (void **)&stmt, OCI_HTYPE_STMT, 0, (dvoid **) 0);
    if (status != OCI_SUCCESS) return FAILURE;

    if (errhp == NULL) {
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log_oci_error("Failed to allocate error handle", log, errhp);
            OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
            OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;
            return FAILURE;
        }
    }

    status = OCIStmtPrepare(stmt, errhp, (text *) "SELECT 1 FROM DUAL",
                            (ub4)strlen("SELECT 1 FROM DUAL"),
                            OCI_NTV_SYNTAX, OCI_DEFAULT);

    if (status != OCI_SUCCESS) {
        log_oci_error("Failed to prepare statement", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT);
        return FAILURE;
    }

    status = OCIStmtExecute(svchp, stmt, errhp, 0, 0, NULL, NULL, OCI_DEFAULT);

    if (status != OCI_SUCCESS) {
        log_oci_error("Failed to execute statement", log, errhp);
        OCIHandleFree(stmt, OCI_HTYPE_STMT);
        return FAILURE;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Ping executed successfully\n");
        log_oci_status(log, status);
    }

    OCIHandleFree(stmt, OCI_HTYPE_STMT); stmt = NULL;

    return SUCCESS;
}