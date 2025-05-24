#include "database.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <yaml.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <log4c.h>
#include "./utils/helper.h"
#include "../utils/helper.h"
#include "../utils/mapping.h"

#include <oci.h>

static int db_initialized = 0;

OCIEnv *envhp = NULL;
OCIError *errhp = NULL;
OCISvcCtx *svchp = NULL;
OCISession *usrhp = NULL;
OCIServer *srvhp = NULL;

// Custom memory allocation function with context
void *custom_malloc(void *ctxp, size_t size) {
    printf("custom_malloc called, ctxp: %p, size: %zu\n", ctxp, size);

    void*ptr = malloc(size);
    if(ptr == NULL) {
        printf("Failed to allocate memory\n");
        return NULL;
    }
    else
        printf("allocated memory of size %zu at %p\n", size, ptr);

    return ptr;
}

// Custom memory reallocation function with context
void *custom_realloc(void *ctxp, void *ptr, size_t size) {
    printf("custom_realloc called, ctxp: %p, ptr: %p, size: %zu\n", ctxp, ptr, size);

    void *new_ptr = realloc(ptr, size);

    if(new_ptr == NULL) {
        printf("Failed to reallocate memory\n");
        return NULL;
    }
    else
        printf("Reallocated memory of size %zu at %p\n", size, new_ptr);

    return new_ptr;
}

// Custom memory free function with context
void custom_free(void *ctxp, void *ptr) {
    printf("Freeing memory of address %p.\n", ptr);
    free(ptr);
}


int init_db(const char *config_file) {

    log4c_category_t* log = initialize_log4c_category("init_db");

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Initializing database\n");

    if (db_initialized) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Database already initialized\n");
        return 0; // Already initialized, singleton implementation
    }

    FILE *file;
    if ((file = fopen(config_file, "r")) == NULL) {
        perror("fopen");
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to open config file: [%s]\n", config_file);
        return -1;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Config file opened\n");

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        fputs("Failed to initialize parser!\n", stderr);
        fclose(file);
        return -2;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Parser initialized\n");

    yaml_parser_set_input_file(&parser, file);

    char username[100], password[100], dbname[100], tns_admin[256];

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(dbname, 0, sizeof(dbname));
    memset(tns_admin, 0, sizeof(tns_admin));

    bool found_username = false;
    bool found_password = false;
    bool found_dbname = false;
    bool found_tns_admin = false;

    bool username_set = false;
    bool password_set = false;
    bool dbname_set = false;
    bool tns_admin_set = false;

    /*
      username: "uninadev"
      password: "Unidevtest4011$"
      dbname: "bpk3181nciydujmq_medium"
      tns_admin: "/usr/local/instantclient/network/admin"



    strcpy(username, "uninadev");
    strcpy(password, "Unidevtest4011$");
    strcpy(dbname, "bpk3181nciydujmq_medium");
    strcpy(tns_admin, "/usr/local/instantclient/network/admin");*/

    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type != YAML_SCALAR_TOKEN) {
            yaml_token_delete(&token);
            continue;
        }

        if (!found_username && strcmp((char *)token.data.scalar.value, "username") == 0) {
            found_username = true;
            yaml_token_delete(&token);
            continue;
        }

        if (found_username && !username_set &&
            token.data.scalar.value != NULL && strlen((char *)token.data.scalar.value) > 0) {

            strncpy(username, (char *)token.data.scalar.value, sizeof(username) - 1);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Username: %s\n", username);
            username_set = true;
            yaml_token_delete(&token);
            continue;
        }

        if(!found_password && strcmp((char *)token.data.scalar.value, "password") == 0) {
            found_password = true;
            yaml_token_delete(&token);
            continue;
        }

        if (found_password && !password_set &&
            token.data.scalar.value != NULL && strlen((char *)token.data.scalar.value) > 0) {

            strncpy(password, (char *)token.data.scalar.value, sizeof(password) - 1);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Password: %s\n", password);
            password_set = true;
            yaml_token_delete(&token);
            continue;
        }

        if(!found_dbname && strcmp((char *)token.data.scalar.value, "dbname") == 0) {
            found_dbname = true;
            yaml_token_delete(&token);
            continue;
        }

        if (found_dbname && !dbname_set &&
            token.data.scalar.value != NULL && strlen((char *)token.data.scalar.value) > 0) {

            strncpy(dbname, (char *)token.data.scalar.value, sizeof(dbname) - 1);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "DB Name: %s\n", dbname);
            dbname_set = true;
            yaml_token_delete(&token);
            continue;
        }

        if(!found_tns_admin && strcmp((char *)token.data.scalar.value, "tns_admin") == 0) {
            found_tns_admin = true;
            yaml_token_delete(&token);
            continue;
        }

        if (found_tns_admin && !tns_admin_set &&
            token.data.scalar.value != NULL && strlen((char *)token.data.scalar.value) > 0) {

            strncpy(tns_admin, (char *)token.data.scalar.value, sizeof(tns_admin) - 1);
            log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "TNS Admin: %s\n", tns_admin);
            tns_admin_set = true;
            yaml_token_delete(&token);
            continue;
        }

        if(username_set && password_set && dbname_set && tns_admin_set) {
            yaml_token_delete(&token);
            break;
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    fclose(file);

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Config file parsed, creating OCI environment.\n");

    if (OCIEnvCreate(&envhp, OCI_DEFAULT | OCI_ENV_NO_MUTEX, NULL, custom_malloc, custom_realloc, custom_free, 0, NULL) != OCI_SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create environment\n");
        envhp = NULL;
        return -3;
    }
    else
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Environment created\n");

    if (OCIHandleAlloc(envhp, (dvoid **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
        OCIHandleFree(envhp, OCI_HTYPE_ENV); // Deallocate environment handle
        envhp = NULL;
        errhp = NULL;
        return -4;
    }
    else
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Error handle allocated\n");

    if (OCIHandleAlloc(envhp, (dvoid**)&srvhp, OCI_HTYPE_SERVER, 0, NULL) != OCI_SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate server handle\n");
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL; // Deallocate error handle
        OCIHandleFree(envhp, OCI_HTYPE_ENV); // Deallocate environment handle
        envhp = NULL;
        errhp = NULL;
        srvhp = NULL;
        return -5;
    }
    else
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server handle allocated\n");

    if (OCIHandleAlloc(envhp, (void **)&svchp, OCI_HTYPE_SVCCTX, 0, NULL) != OCI_SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate service context handle\n");

        OCIHandleFree(srvhp, OCI_HTYPE_SERVER); // Deallocate server handle
        OCIHandleFree(envhp, OCI_HTYPE_ENV); // Deallocate environment handle
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL; // Deallocate error handle
        envhp = NULL;
        errhp = NULL;
        srvhp = NULL;
        svchp = NULL;
        return -6;
    }
    else
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Service context handle allocated\n");

    // Set TNS_ADMIN environment variable
    setenv("TNS_ADMIN", tns_admin, 1);
    setenv("TRACE_LEVEL_CLIENT", "16", 1); // Set the trace level to a high value for detailed tracing

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Attempting to connect to the database\n");

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "envhp: %p, errhp: %p, srvhp: %p, svchp: %p\n", (void*)envhp, (void*)errhp, (void*)srvhp, (void*)svchp);

    if(OCILogon2(envhp, errhp, &svchp,
                (const OraText *)username, strlen(username),
                (const OraText *)password, strlen(password),
                (const OraText *)dbname, strlen(dbname),
                OCI_DEFAULT) != OCI_SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to connect to the database\n");

        OCIHandleFree(srvhp, OCI_HTYPE_SERVER); // Deallocate server handle
        OCIHandleFree(envhp, OCI_HTYPE_ENV); // Deallocate environment handle
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL; // Deallocate error handle
        OCIHandleFree(svchp, OCI_HTYPE_SVCCTX); // Deallocate service context handle
        envhp = NULL;
        errhp = NULL;
        srvhp = NULL;
        svchp = NULL;

        return -7;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Connected to the database\n");
    db_initialized = 1;
    return 0;
}

void close_db() {
    log4c_category_t* log = initialize_log4c_category("close_db");
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Closing database connection\n");

    if (!db_initialized) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Database not initialized, nothing to close\n");
        return;
    }

    if(errhp == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Allocating error handle\n");
        if (OCIHandleAlloc(envhp, (void **)&errhp, OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate error handle\n");
            return;
        }
    }

    // Logoff first using OCILogoff if you used OCILogon2
    if (svchp != NULL) {
        if (OCILogoff(svchp, errhp) != OCI_SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to log off from the database\n");
        }
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Service context handle is NULL, skipping logoff\n");
    }

    // Terminate the session
    if (usrhp) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Ending session\n");
        OCISessionEnd(svchp, errhp, usrhp, OCI_DEFAULT);
        OCIHandleFree(usrhp, OCI_HTYPE_SESSION);
        usrhp = NULL;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Session handle is NULL, skipping session end\n");
    }

    // Detach from server
    if (srvhp) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Detaching from server\n");
        OCIServerDetach(srvhp, errhp, OCI_DEFAULT);
        OCIHandleFree(srvhp, OCI_HTYPE_SERVER);
        srvhp = NULL;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Server handle is NULL, skipping server detach\n");
    }

    // Free service context handle
    if (svchp) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Freeing service context handle\n");
        OCIHandleFree(svchp, OCI_HTYPE_SVCCTX);
        svchp = NULL;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Service context handle is NULL, skipping service context free\n");
    }

    // Free error handle
    if (errhp) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Freeing error handle\n");
        OCIHandleFree(errhp, OCI_HTYPE_ERROR); errhp = NULL;
        errhp = NULL;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Error handle is NULL, skipping error handle free\n");
    }

    // Free environment handle
    if (envhp) {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Freeing environment handle\n");
        OCIHandleFree(envhp, OCI_HTYPE_ENV);
        envhp = NULL;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Environment handle is NULL, skipping environment handle free\n");
    }

    db_initialized = 0;

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Database connection closed\n");
}

int terminate_db() {
    log4c_category_t* log = initialize_log4c_category("terminate_db");

    // Call OCITerminate to clean up remaining resources
    if (OCITerminate(OCI_DEFAULT) != OCI_SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to terminate OCI");
        return FAILURE;
    }
    else {
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "OCI terminated successfully\n");
    }

    return SUCCESS;
}

int check_db_connection() {
    log4c_category_t* log = initialize_log4c_category("check_db_connection");
    if (!db_initialized) {
        db_initialized = true;
    }

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Checking database connection...");

    // Call a lightweight query to verify the connection is alive
    // Example: SELECT 1 FROM DUAL for Oracle
    int res = execute_db_ping();

    if (res != SUCCESS) {
        log4c_category_log(log, LOG4C_PRIORITY_WARN,
                           "Database connection check failed (res=%d), attempting to reconnect", res);

        // Close the current connection, clean up resources
        close_db();
        log4c_category_log(log, LOG4C_PRIORITY_DEBUG, "Close Db called\n");

        // Wait for a short period before attempting to reconnect
        sleep(1);

        // Attempt to reconnect
        res = init_db("resources/config/config.yaml");

        if (res != SUCCESS) {
            log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to reconnect to database (res=%d)", res);
            return FAILURE;
        }
        else
            log4c_category_log(log, LOG4C_PRIORITY_INFO, "Successfully reconnected to database");
    }

    return res;
}
