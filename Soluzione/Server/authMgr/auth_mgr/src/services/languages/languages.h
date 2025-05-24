#ifndef AUTH_MGR_SRC_SERVICES_LANGUAGES_LANGUAGES_H_
#define AUTH_MGR_SRC_SERVICES_LANGUAGES_LANGUAGES_H_

#include "../../entities/lingua.h"

int ottieni_lingua(const char *codice_lingua, char* descrizione_lingua);
int ottieni_lingue(lingua** lingue, int* num_lingue);

#endif //AUTH_MGR_SRC_SERVICES_LANGUAGES_LANGUAGES_H_
