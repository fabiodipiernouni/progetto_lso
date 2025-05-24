#ifndef DATABASE_H
#define DATABASE_H

#include <oci.h>

extern OCIEnv *envhp;
extern OCIError *errhp;
extern OCISvcCtx *svchp;
extern OCISession *usrhp;
extern OCIServer *srvhp;

int init_db(const char *config_file);
void close_db();
int check_db_connection();
int terminate_db();

#endif // DATABASE_H