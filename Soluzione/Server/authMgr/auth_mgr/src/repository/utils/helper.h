#ifndef AUTH_MGR_SRC_REPOSITORY_UTILS_HELPER_H_
#define AUTH_MGR_SRC_REPOSITORY_UTILS_HELPER_H_

#include <oci.h>
#include <log4c.h>

int log_oci_error(const char* message, log4c_category_t*log, OCIError *errhp);
void log_oci_status(log4c_category_t* log, sword status);
int execute_db_ping();

#endif //AUTH_MGR_SRC_REPOSITORY_UTILS_HELPER_H_
