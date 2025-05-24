#ifndef AUTHMGR_AUTH_MGR_SRC_REPOSITORY_LINGUE_LINGUE_H_
#define AUTHMGR_AUTH_MGR_SRC_REPOSITORY_LINGUE_LINGUE_H_

#include "../../entities/lingua.h"

int get_descrizione_lingua(const char *codice_lingua, char *descrizione_lingua);
int get_lingue(lingua** lingue, int* num_lingue);

#endif //AUTHMGR_AUTH_MGR_SRC_REPOSITORY_LINGUE_LINGUE_H_
