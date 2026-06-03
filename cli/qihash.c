#include "qihash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void usage(void) {
    printf("%s\n", qihash_version_string());
    printf("\n");
    printf("Usage:\n");
    printf("  qihash hash [--out BYTES] [--file PATH | TEXT]\n");
    printf("  qihash kdf --password TEXT --salt HEX [--pepper HEX] [--device HEX] [--memory KiB] [--passes N] [--mode id|safe|hard] [--out BYTES]\n");
    printf("  qihash password-hash --password TEXT [--profile test|light|web|admin|archive] [--pepper HEX] [--device HEX] [--memory KiB] [--passes N] [--mode id|safe|hard] [--out BYTES]\n");
    printf("  qihash password-verify --password TEXT --encoded HASH [--pepper HEX] [--device HEX]\n");
    printf("  qihash bench [--profile test|light|web|admin|archive] [--memory KiB] [--passes N]\n");
    printf("  qihash selftest\n");
    printf("\n");
    printf("Examples:\n");
    printf("  qihash hash hello\n");
    printf("  qihash hash --file ./README.md --out 64\n");
    printf("  qihash password-hash --password secret --profile test\n");
    printf("  qihash password-verify --password secret --encoded '$qihash$...'\n");
}

static const char *arg_value(int *i, int argc, char **argv) {
    if (*i + 1 >= argc) return NULL;
    (*i)++;
    return argv[*i];
}

static int parse_u32(const char *s, uint32_t *out) {
    if (!s || !*s) return QIHASH_ERR_PARAM;
    char *end = NULL;
    unsigned long v = strtoul(s, &end, 10);
    if (!end || *end || v > 0xffffffffUL) return QIHASH_ERR_PARAM;
    *out = (uint32_t)v;
    return QIHASH_OK;
}

static int parse_hex_optional(const char *hex, uint8_t **out, size_t *out_len) {
    *out = NULL; *out_len = 0;
    if (!hex) return QIHASH_OK;
    size_t cap = strlen(hex) / 2 + 1;
    uint8_t *buf = (uint8_t *)malloc(cap);
    if (!buf) return QIHASH_ERR_MEMORY;
    size_t n = cap;
    int r = qihash_hex_decode(hex, buf, &n);
    if (r != QIHASH_OK) { free(buf); return r; }
    *out = buf; *out_len = n;
    return QIHASH_OK;
}

static int print_hex_result(const uint8_t *buf, size_t len) {
    char *hex = (char *)malloc(len * 2 + 1);
    if (!hex) return QIHASH_ERR_MEMORY;
    int r = qihash_hex_encode(buf, len, hex, len * 2 + 1);
    if (r == QIHASH_OK) printf("%s\n", hex);
    qihash_secure_zero(hex, len * 2 + 1);
    free(hex);
    return r;
}

static int cmd_hash(int argc, char **argv) {
    const char *file = NULL;
    const char *text = NULL;
    uint32_t out_len = QIHASH_HASH_BYTES_DEFAULT;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--file") == 0) file = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--out") == 0) {
            const char *v = arg_value(&i, argc, argv);
            if (parse_u32(v, &out_len) != QIHASH_OK) return QIHASH_ERR_PARAM;
        } else if (!text) text = argv[i];
        else return QIHASH_ERR_PARAM;
    }
    if (file && text) return QIHASH_ERR_PARAM;
    if (!file && !text) text = "";
    uint8_t out[QIHASH_HASH_BYTES_MAX];
    int r = file ? qihash_hash_file(file, out, out_len) : qihash_hash(text, strlen(text), out, out_len);
    if (r != QIHASH_OK) return r;
    r = print_hex_result(out, out_len);
    qihash_secure_zero(out, sizeof(out));
    return r;
}

static int cmd_kdf(int argc, char **argv) {
    const char *password = NULL, *salt_hex = NULL, *pepper_hex = NULL, *device_hex = NULL;
    qihash_params p; qihash_params_default(&p);
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--password") == 0) password = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--salt") == 0) salt_hex = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--pepper") == 0) pepper_hex = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--device") == 0) device_hex = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--memory") == 0) { const char *v = arg_value(&i, argc, argv); if (parse_u32(v, &p.memory_kib) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--passes") == 0) { const char *v = arg_value(&i, argc, argv); if (parse_u32(v, &p.passes) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--out") == 0) { const char *v = arg_value(&i, argc, argv); if (parse_u32(v, &p.output_len) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--mode") == 0) { const char *v = arg_value(&i, argc, argv); if (qihash_mode_from_name(v, &p.mode) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else return QIHASH_ERR_PARAM;
    }
    if (!password || !salt_hex) return QIHASH_ERR_PARAM;
    uint8_t *salt = NULL, *pepper = NULL, *device = NULL;
    size_t salt_len = 0, pepper_len = 0, device_len = 0;
    int r = parse_hex_optional(salt_hex, &salt, &salt_len);
    if (r != QIHASH_OK) goto done;
    r = parse_hex_optional(pepper_hex, &pepper, &pepper_len);
    if (r != QIHASH_OK) goto done;
    r = parse_hex_optional(device_hex, &device, &device_len);
    if (r != QIHASH_OK) goto done;
    uint8_t *out = (uint8_t *)malloc(p.output_len);
    if (!out) { r = QIHASH_ERR_MEMORY; goto done; }
    r = qikdf((const uint8_t *)password, strlen(password), salt, salt_len, pepper, pepper_len, device, device_len, &p, out, p.output_len);
    if (r == QIHASH_OK) r = print_hex_result(out, p.output_len);
    qihash_secure_zero(out, p.output_len); free(out);
done:
    if (salt) { qihash_secure_zero(salt, salt_len); free(salt); }
    if (pepper) { qihash_secure_zero(pepper, pepper_len); free(pepper); }
    if (device) { qihash_secure_zero(device, device_len); free(device); }
    return r;
}

static int apply_common_params(int argc, char **argv, qihash_params *p) {
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--profile") == 0) {
            const char *v = arg_value(&i, argc, argv);
            if (qihash_params_profile(v, p) != QIHASH_OK) return QIHASH_ERR_PARAM;
        }
    }
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--memory") == 0) { const char *v = arg_value(&i, argc, argv); if (parse_u32(v, &p->memory_kib) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--passes") == 0) { const char *v = arg_value(&i, argc, argv); if (parse_u32(v, &p->passes) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--mode") == 0) { const char *v = arg_value(&i, argc, argv); if (qihash_mode_from_name(v, &p->mode) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--out") == 0) { const char *v = arg_value(&i, argc, argv); if (parse_u32(v, &p->output_len) != QIHASH_OK) return QIHASH_ERR_PARAM; }
        else if (strcmp(argv[i], "--profile") == 0) { (void)arg_value(&i, argc, argv); }
        else if (strcmp(argv[i], "--password") == 0 || strcmp(argv[i], "--pepper") == 0 || strcmp(argv[i], "--device") == 0 || strcmp(argv[i], "--encoded") == 0) { (void)arg_value(&i, argc, argv); }
        else return QIHASH_ERR_PARAM;
    }
    return qihash_params_validate(p);
}

static int cmd_password_hash(int argc, char **argv) {
    const char *password = NULL, *pepper_hex = NULL, *device_hex = NULL;
    qihash_params p; qihash_params_default(&p);
    int r = apply_common_params(argc, argv, &p);
    if (r != QIHASH_OK) return r;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--password") == 0) password = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--pepper") == 0) pepper_hex = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--device") == 0) device_hex = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--profile") == 0 || strcmp(argv[i], "--memory") == 0 || strcmp(argv[i], "--passes") == 0 || strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "--out") == 0) (void)arg_value(&i, argc, argv);
    }
    if (!password) return QIHASH_ERR_PARAM;
    uint8_t *pepper = NULL, *device = NULL; size_t pepper_len = 0, device_len = 0;
    r = parse_hex_optional(pepper_hex, &pepper, &pepper_len); if (r != QIHASH_OK) goto done;
    r = parse_hex_optional(device_hex, &device, &device_len); if (r != QIHASH_OK) goto done;
    char encoded[QIHASH_ENCODED_MAX];
    r = qihash_password_hash(password, pepper, pepper_len, device, device_len, &p, encoded, sizeof(encoded));
    if (r == QIHASH_OK) printf("%s\n", encoded);
done:
    if (pepper) { qihash_secure_zero(pepper, pepper_len); free(pepper); }
    if (device) { qihash_secure_zero(device, device_len); free(device); }
    return r;
}

static int cmd_password_verify(int argc, char **argv) {
    const char *password = NULL, *encoded = NULL, *pepper_hex = NULL, *device_hex = NULL;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--password") == 0) password = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--encoded") == 0) encoded = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--pepper") == 0) pepper_hex = arg_value(&i, argc, argv);
        else if (strcmp(argv[i], "--device") == 0) device_hex = arg_value(&i, argc, argv);
        else return QIHASH_ERR_PARAM;
    }
    if (!password || !encoded) return QIHASH_ERR_PARAM;
    uint8_t *pepper = NULL, *device = NULL; size_t pepper_len = 0, device_len = 0;
    int r = parse_hex_optional(pepper_hex, &pepper, &pepper_len); if (r != QIHASH_OK) goto done;
    r = parse_hex_optional(device_hex, &device, &device_len); if (r != QIHASH_OK) goto done;
    r = qihash_password_verify(password, encoded, pepper, pepper_len, device, device_len);
    printf("%s\n", r == QIHASH_OK ? "OK" : "FAIL");
done:
    if (pepper) { qihash_secure_zero(pepper, pepper_len); free(pepper); }
    if (device) { qihash_secure_zero(device, device_len); free(device); }
    return r;
}

static double now_seconds(void) {
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

static int cmd_bench(int argc, char **argv) {
    qihash_params p; qihash_params_profile("test", &p);
    int r = apply_common_params(argc, argv, &p);
    if (r != QIHASH_OK) return r;
    uint8_t salt[32];
    uint8_t out[QIHASH_HASH_BYTES_MAX];
    memset(salt, 0x42, sizeof(salt));
    double a = now_seconds();
    r = qikdf((const uint8_t *)"benchmark-password", 18, salt, sizeof(salt), NULL, 0, NULL, 0, &p, out, p.output_len);
    double b = now_seconds();
    if (r != QIHASH_OK) return r;
    printf("profile: custom\nmode: %s\nmemory_kib: %u\npasses: %u\nout: %u\ntime_sec: %.6f\n",
           qihash_mode_name(p.mode), p.memory_kib, p.passes, p.output_len, b - a);
    print_hex_result(out, p.output_len);
    qihash_secure_zero(out, sizeof(out));
    return QIHASH_OK;
}

static int cmd_selftest(void) {
    uint8_t h1[64], h2[64], h3[64];
    int r = qihash_hash("abc", 3, h1, sizeof(h1));
    if (r != QIHASH_OK) return r;
    r = qihash_hash("abc", 3, h2, sizeof(h2));
    if (r != QIHASH_OK) return r;
    r = qihash_hash("abd", 3, h3, sizeof(h3));
    if (r != QIHASH_OK) return r;
    if (!qihash_constant_time_equal(h1, h2, sizeof(h1))) return QIHASH_ERR_VERIFY;
    if (qihash_constant_time_equal(h1, h3, sizeof(h1))) return QIHASH_ERR_VERIFY;

    qihash_params p; qihash_params_profile("test", &p);
    uint8_t pepper[16]; memset(pepper, 0x11, sizeof(pepper));
    uint8_t device[16]; memset(device, 0x22, sizeof(device));
    char encoded[QIHASH_ENCODED_MAX];
    r = qihash_password_hash("correct horse battery staple", pepper, sizeof(pepper), device, sizeof(device), &p, encoded, sizeof(encoded));
    if (r != QIHASH_OK) return r;
    r = qihash_password_verify("correct horse battery staple", encoded, pepper, sizeof(pepper), device, sizeof(device));
    if (r != QIHASH_OK) return r;
    r = qihash_password_verify("wrong", encoded, pepper, sizeof(pepper), device, sizeof(device));
    if (r != QIHASH_ERR_VERIFY) return QIHASH_ERR_VERIFY;
    printf("selftest: OK\n");
    return QIHASH_OK;
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        usage();
        return 0;
    }
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "version") == 0) {
        printf("%s\n", qihash_version_string());
        return 0;
    }
    int r = QIHASH_ERR_PARAM;
    if (strcmp(argv[1], "hash") == 0) r = cmd_hash(argc, argv);
    else if (strcmp(argv[1], "kdf") == 0) r = cmd_kdf(argc, argv);
    else if (strcmp(argv[1], "password-hash") == 0) r = cmd_password_hash(argc, argv);
    else if (strcmp(argv[1], "password-verify") == 0) r = cmd_password_verify(argc, argv);
    else if (strcmp(argv[1], "bench") == 0) r = cmd_bench(argc, argv);
    else if (strcmp(argv[1], "selftest") == 0) r = cmd_selftest();
    else usage();
    if (r != QIHASH_OK) {
        fprintf(stderr, "qihash: %s\n", qihash_error_string(r));
        return r == QIHASH_ERR_VERIFY ? 2 : 1;
    }
    return 0;
}
