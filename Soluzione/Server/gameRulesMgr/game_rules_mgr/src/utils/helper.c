#include "helper.h"
#include <log4c.h>
#include "../entities/utente.h"
#include "../entities/stanza.h"

const char* log_level_strings[] = {
    "FATAL",
    "ALERT",
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "TRACE"
};

log4c_priority_level_t get_log4c_level(const log_levels log_level) {
    switch (log_level) {
        case FATAL:
            return LOG4C_PRIORITY_FATAL;
        case ALERT:
            return LOG4C_PRIORITY_ALERT;
        case CRIT:
            return LOG4C_PRIORITY_CRIT;
        case ERROR:
            return LOG4C_PRIORITY_ERROR;
        case WARN:
            return LOG4C_PRIORITY_WARN;
        case NOTICE:
            return LOG4C_PRIORITY_NOTICE;
        case INFO:
            return LOG4C_PRIORITY_INFO;
        case DEBUG:
            return LOG4C_PRIORITY_DEBUG;
        case TRACE:
            return LOG4C_PRIORITY_TRACE;
        default:
            return LOG4C_PRIORITY_INFO;
    }
}

// Function to convert enum log_levels to string
const char* log_level_to_string(log_levels level) {
    if (level >= 0 && level < LOG_LEVEL_COUNT) {
        return log_level_strings[level];
    }
    return "UNKNOWN";
}

// Function to convert string to enum log_levels
log_levels string_to_log_level(const char* str) {
    for (int i = 0; i < LOG_LEVEL_COUNT; ++i) {
        if (strcmp(str, log_level_strings[i]) == 0) {
            return (log_levels)i;
        }
    }
    return LOG_LEVEL_COUNT; // Return an invalid value if not found
}

// Function to convert yaml_token_type_t to string
const char* yaml_token_type_to_string(yaml_token_type_t type) {
    switch (type) {
        case YAML_NO_TOKEN:
            return "YAML_NO_TOKEN";
        case YAML_STREAM_START_TOKEN:
            return "YAML_STREAM_START_TOKEN";
        case YAML_STREAM_END_TOKEN:
            return "YAML_STREAM_END_TOKEN";
        case YAML_VERSION_DIRECTIVE_TOKEN:
            return "YAML_VERSION_DIRECTIVE_TOKEN";
        case YAML_TAG_DIRECTIVE_TOKEN:
            return "YAML_TAG_DIRECTIVE_TOKEN";
        case YAML_DOCUMENT_START_TOKEN:
            return "YAML_DOCUMENT_START_TOKEN";
        case YAML_DOCUMENT_END_TOKEN:
            return "YAML_DOCUMENT_END_TOKEN";
        case YAML_BLOCK_SEQUENCE_START_TOKEN:
            return "YAML_BLOCK_SEQUENCE_START_TOKEN";
        case YAML_BLOCK_MAPPING_START_TOKEN:
            return "YAML_BLOCK_MAPPING_START_TOKEN";
        case YAML_BLOCK_END_TOKEN:
            return "YAML_BLOCK_END_TOKEN";
        case YAML_FLOW_SEQUENCE_START_TOKEN:
            return "YAML_FLOW_SEQUENCE_START_TOKEN";
        case YAML_FLOW_SEQUENCE_END_TOKEN:
            return "YAML_FLOW_SEQUENCE_END_TOKEN";
        case YAML_FLOW_MAPPING_START_TOKEN:
            return "YAML_FLOW_MAPPING_START_TOKEN";
        case YAML_FLOW_MAPPING_END_TOKEN:
            return "YAML_FLOW_MAPPING_END_TOKEN";
        case YAML_BLOCK_ENTRY_TOKEN:
            return "YAML_BLOCK_ENTRY_TOKEN";
        case YAML_FLOW_ENTRY_TOKEN:
            return "YAML_FLOW_ENTRY_TOKEN";
        case YAML_KEY_TOKEN:
            return "YAML_KEY_TOKEN";
        case YAML_VALUE_TOKEN:
            return "YAML_VALUE_TOKEN";
        case YAML_ALIAS_TOKEN:
            return "YAML_ALIAS_TOKEN";
        case YAML_ANCHOR_TOKEN:
            return "YAML_ANCHOR_TOKEN";
        case YAML_TAG_TOKEN:
            return "YAML_TAG_TOKEN";
        case YAML_SCALAR_TOKEN:
            return "YAML_SCALAR_TOKEN";
        default:
            return "UNKNOWN_TOKEN";
    }
}

// Helper function to initialize a log4c category
log4c_category_t* initialize_log4c_category(const char* category_name) {
    if (!category_name) {
        fprintf(stderr, "log category_name not valid.\n");
        return NULL;
    }

    log4c_category_t* log = log4c_category_get(category_name);
    if (!log) {
        fprintf(stderr, "Cannot obtain category '%s'\n", category_name);
        return NULL;
    }

    // Set the priority level for the category
    log4c_category_set_priority(log, LOG4C_PRIORITY_DEBUG);

    // Get the console appender
    log4c_appender_t* appender = log4c_appender_get("stdout");
    log4c_layout_t* layout = log4c_layout_get("dated");

    // Associa il layout all'appender
    log4c_appender_set_layout(appender, layout);


    // Set the appender for the category
    log4c_category_set_appender(log, appender);

    return log;
}

void log_user(const Utente* user, const log4c_category_t* log) {
    log4c_category_log(log, LOG4C_PRIORITY_DEBUG,
        "User id: %d\n"
        "User name: %s\n"
        "User surname: %s\n"
        "User email: %s\n"
        "User lang code: %s\n",
        user->id_utente, user->nome, user->cognome, user->email, user->codice_lingua
    );
}

void log_room(const Stanza* room, const log4c_category_t* log) {

    log4c_category_log(log, LOG4C_PRIORITY_DEBUG,
        "Room id: %d\n"
        "Room name: %s\n"
        "Room game order: %d\n"
        "Room state: %d\n",
        room->indice_stanza, room->nome,
        room->ordine_gioco, room->stato_gioco
    );

}