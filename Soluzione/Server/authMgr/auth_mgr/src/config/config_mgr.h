#ifndef PRJ_SERVICE_CONFIG_MGR_H_
#define PRJ_SERVICE_CONFIG_MGR_H_

#include <log4c.h>
#include "../utils/helper.h"

log4c_priority_level_t get_log4c_level(const log_levels log_level);
int get_port_from_config(const char *config_file);
int log_setup(const char *config_file, log4c_category_t* category);

#endif //PRJ_SERVICE_CONFIG_MGR_H_
