#include "qihash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int test_hash_deterministic(void) {
    uint8_t a[64], b[64], c[64];
    if (qihash_hash("hello", 5, a, sizeof(a)) != QIHASH_OK) return 1;
    if (qihash_hash("hello", 5, b, sizeof(b)) != QIHASH_OK) return 2;
    if (qihash_hash("Hello", 5, c, sizeof(c)) != QIHASH_OK) return 3;
    if (!qihash_constant_time_equal(a, b, sizeof(a))) return 4;
    if (qihash_constant_time_equal(a, c, sizeof(a))) return 5;
    return 0;
}

static int test_b64(void) {
    const uint8_t raw[] = {0, 1, 2, 3, 250, 251, 252, 253, 254, 255};
    char enc[64];
    uint8_t dec[64]; size_t dec_len = sizeof(dec);
    if (qihash_b64url_encode(raw, sizeof(raw), enc, sizeof(enc)) != QIHASH_OK) return 1;
    if (qihash_b64url_decode(enc, dec, &dec_len) != QIHASH_OK) return 2;
    if (dec_len != sizeof(raw)) return 3;
    if (memcmp(raw, dec, sizeof(raw)) != 0) return 4;
    return 0;
}

static int test_hex(void) {
    const uint8_t raw[] = {0xde, 0xad, 0xbe, 0xef};
    char hex[16];
    uint8_t dec[4]; size_t dec_len = sizeof(dec);
    if (qihash_hex_encode(raw, sizeof(raw), hex, sizeof(hex)) != QIHASH_OK) return 1;
    if (strcmp(hex, "deadbeef") != 0) return 2;
    if (qihash_hex_decode(hex, dec, &dec_len) != QIHASH_OK) return 3;
    if (dec_len != sizeof(raw) || memcmp(raw, dec, sizeof(raw)) != 0) return 4;
    return 0;
}

static int test_kdf_deterministic(void) {
    qihash_params p;
    qihash_params_profile("test", &p);
    uint8_t salt[32]; memset(salt, 0x33, sizeof(salt));
    uint8_t pepper[16]; memset(pepper, 0x44, sizeof(pepper));
    uint8_t a[64], b[64], c[64];
    if (qikdf((const uint8_t *)"pass", 4, salt, sizeof(salt), pepper, sizeof(pepper), NULL, 0, &p, a, sizeof(a)) != QIHASH_OK) return 1;
    if (qikdf((const uint8_t *)"pass", 4, salt, sizeof(salt), pepper, sizeof(pepper), NULL, 0, &p, b, sizeof(b)) != QIHASH_OK) return 2;
    if (qikdf((const uint8_t *)"pass2", 5, salt, sizeof(salt), pepper, sizeof(pepper), NULL, 0, &p, c, sizeof(c)) != QIHASH_OK) return 3;
    if (!qihash_constant_time_equal(a, b, sizeof(a))) return 4;
    if (qihash_constant_time_equal(a, c, sizeof(a))) return 5;
    return 0;
}

static int test_password_hash_verify(void) {
    qihash_params p;
    qihash_params_profile("test", &p);
    uint8_t pepper[16]; memset(pepper, 0x55, sizeof(pepper));
    uint8_t device[16]; memset(device, 0x66, sizeof(device));
    char encoded[QIHASH_ENCODED_MAX];
    if (qihash_password_hash("secret", pepper, sizeof(pepper), device, sizeof(device), &p, encoded, sizeof(encoded)) != QIHASH_OK) return 1;
    if (qihash_password_verify("secret", encoded, pepper, sizeof(pepper), device, sizeof(device)) != QIHASH_OK) return 2;
    if (qihash_password_verify("bad", encoded, pepper, sizeof(pepper), device, sizeof(device)) != QIHASH_ERR_VERIFY) return 3;
    return 0;
}

static int run(const char *name, int (*fn)(void)) {
    int r = fn();
    printf("%-28s %s\n", name, r == 0 ? "OK" : "FAIL");
    if (r) printf("  code: %d\n", r);
    return r;
}

int main(void) {
    int fail = 0;
    fail |= run("hash deterministic", test_hash_deterministic);
    fail |= run("base64url", test_b64);
    fail |= run("hex", test_hex);
    fail |= run("kdf deterministic", test_kdf_deterministic);
    fail |= run("password hash verify", test_password_hash_verify);
    return fail ? 1 : 0;
}
