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

    uint8_t decoded[4096];
    size_t decoded_len = sizeof(decoded);
    (void)qihash_hex_decode(input, decoded, &decoded_len);

    decoded_len = sizeof(decoded);
    (void)qihash_b64url_decode(input, decoded, &decoded_len);

    char encoded[8192];
    (void)qihash_hex_encode(data, size, encoded, sizeof(encoded));
    (void)qihash_b64url_encode(data, size, encoded, sizeof(encoded));

    qihash_secure_zero(decoded, sizeof(decoded));
    free(input);
    return 0;
}
