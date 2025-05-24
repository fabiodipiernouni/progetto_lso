#include "security.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <log4c.h>
#include <string.h>

#include "../../utils/mapping.h"
#include "../../utils/helper.h"

unsigned char salt[16] = {0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f, 0x7a, 0x8b, 0x9c, 0xad, 0xbe, 0xcf, 0xda, 0xeb, 0xfc, 0x0d}; // Example salt value

int string_to_hash_hex(const char *string, char **hash_hex) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    log4c_category_t* log = initialize_log4c_category("string_to_hash");

    // Hash the password with the salt
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to create EVP_MD_CTX\n");
        return -1;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(mdctx, salt, sizeof(salt)) != 1 ||
        EVP_DigestUpdate(mdctx, string, strlen(string)) != 1 ||
        EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1)
    {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to hash password\n");
        EVP_MD_CTX_free(mdctx);
        return -2;
    }

    EVP_MD_CTX_free(mdctx);

    // Convert hash to hex string
    //char hash_hex[2 * hash_len + 1];
    *hash_hex = (char *)malloc(2 * hash_len + 1);

    if (*hash_hex == NULL) {
        log4c_category_log(log, LOG4C_PRIORITY_ERROR, "Failed to allocate memory for hash_hex\n");
        return -3;
    }

    memset(*hash_hex, 0, 2 * hash_len + 1);

    for (unsigned int i = 0; i < hash_len; i++) {
        sprintf((*hash_hex) + (2 * i), "%02x", hash[i]);
    }

    return SUCCESS;
}


