#ifndef GAME_RULES_MGR_SRC_UTILS_HELPER_H_
#define GAME_RULES_MGR_SRC_UTILS_HELPER_H_

#include <log4c.h>
#include <yaml.h>
#include "../entities/utente.h"
#include "../entities/stanza.h"

typedef enum {
    FATAL = 0,
    ALERT = 1,
    CRIT = 2,
    ERROR = 3,
    WARN = 4,
    NOTICE = 5,
    INFO = 6,
    DEBUG = 7,
    TRACE = 8,
    LOG_LEVEL_COUNT = 9
} log_levels;


extern const char* log_level_strings[];

const char* log_level_to_string(log_levels level);
log_levels string_to_log_level(const char* str);
log4c_priority_level_t get_log4c_level(const log_levels log_level);

log4c_category_t* initialize_log4c_category(const char* category_name);
void log_user(const Utente* user, const log4c_category_t* log);
void log_room(const Stanza* room, const log4c_category_t* log);
const char* yaml_token_type_to_string(yaml_token_type_t type);

#endif //GAME_RULES_MGR_SRC_UTILS_HELPER_H_
