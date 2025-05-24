#ifndef TRANSLATOR_SRV_SRC_SERVICES_TRANSLATOR_TRANSLATOR_H_
#define TRANSLATOR_SRV_SRC_SERVICES_TRANSLATOR_TRANSLATOR_H_

int translate(const char *original_msg, char *error_msg, char *language_code_from, char *language_code_to, char *translated_msg);

#endif //TRANSLATOR_SRV_SRC_SERVICES_TRANSLATOR_TRANSLATOR_H_
