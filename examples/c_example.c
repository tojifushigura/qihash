#include "qihash.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    qihash_params params;
    qihash_params_profile("test", &params);

    const uint8_t pepper[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };

    char encoded[QIHASH_ENCODED_MAX];
    int r = qihash_password_hash("secret", pepper, sizeof(pepper), NULL, 0, &params, encoded, sizeof(encoded));
    if (r != QIHASH_OK) {
        fprintf(stderr, "hash failed: %s\n", qihash_error_string(r));
        return 1;
    }

    printf("encoded: %s\n", encoded);
    r = qihash_password_verify("secret", encoded, pepper, sizeof(pepper), NULL, 0);
    printf("verify: %s\n", r == QIHASH_OK ? "OK" : "FAIL");
    return r == QIHASH_OK ? 0 : 1;
}
