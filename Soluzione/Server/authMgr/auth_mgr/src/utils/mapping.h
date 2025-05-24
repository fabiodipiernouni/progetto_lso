#ifndef PRJ_SERVICE_SERVICE_H_
#define PRJ_SERVICE_SERVICE_H_

#include <cjson/cJSON.h>

#define SUCCESS 0
#define NO_DATA_FOUND 1
#define FAILURE 2
#define EXIT_COMMAND -11

// Password not valid
#define WRONG_PASSWORD -12


#define WRONG_EMAIL -13
#define DB_CONNECTION_ERROR -14

// Validations

#define INVALID_JSON -10
#define EMPTY_FIELD -14
#define MISSING_FIELD -15
#define INVALID_FIELD_TYPE -16

// Password not compliant
#define PASSWORD_TOO_SHORT -17
#define PASSWORD_NO_LOWERCASE -18
#define PASSWORD_NO_UPPERCASE -19
#define PASSWORD_NO_DIGIT -20

#define INVALID_FORMAT -21
#define EMAIL_ALREADY_USED -22

void get_invalid_password_msg(int res, char *cause);

#endif //PRJ_SERVICE_SERVICE_H_
