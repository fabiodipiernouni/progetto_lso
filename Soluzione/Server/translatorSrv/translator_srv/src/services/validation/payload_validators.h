#ifndef AUTH_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_
#define AUTH_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_

int validate_translate_payload(const char *payload, char*error_msg, char *language_code_from, char *language_code_to, char *original_msg);

#endif //AUTH_MGR_SRC_SERVICES_VALIDATION_PAYLOAD_VALIDATORS_H_
