#include "qihash.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096) return 0;

    char *input = (char *)malloc(size + 1);
    if (!input) return 0;
    memcpy(input, data, size);
    input[size] = '\0';

    qihash_params params;
    uint8_t salt[128];
    uint8_t hash[256];
    size_t salt_len = sizeof(salt);
    size_t hash_len = sizeof(hash);

    (void)qihash_parse_encoded(input, &params, salt, &salt_len, hash, &hash_len);

    qihash_secure_zero(salt, sizeof(salt));
    qihash_secure_zero(hash, sizeof(hash));
    free(input);
    return 0;
}
