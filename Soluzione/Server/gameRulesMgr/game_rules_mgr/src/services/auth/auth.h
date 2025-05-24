#ifndef GAME_RULES_MGR_SRC_SERVICES_AUTH_H
#define GAME_RULES_MGR_SRC_SERVICES_AUTH_H

#include "../../utils/app_context.h"

int get_user_by_email(AppContext* app_ctx, char email[], const int indice_client);

#endif //GAME_RULES_MGR_SRC_SERVICES_AUTH_H
