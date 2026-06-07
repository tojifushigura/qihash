#ifndef QIHASH_H
#define QIHASH_H

/*
 * qiHash - Quantum Integrity Hashing Framework
 * C99 API for qiHash and qiKDF.
 *
 * Project release version: 0.1.0
 * Encoded format version: v=1
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QIHASH_VERSION_MAJOR 0
#define QIHASH_VERSION_MINOR 1
#define QIHASH_VERSION_PATCH 0

#define QIHASH_SALT_BYTES_DEFAULT 32
#define QIHASH_HASH_BYTES_DEFAULT 64
#define QIHASH_HASH_BYTES_MIN 16
#define QIHASH_HASH_BYTES_MAX 128
#define QIHASH_MIN_MEMORY_KIB 64
#define QIHASH_MAX_MEMORY_KIB (1024u * 1024u * 16u) /* 16 TiB safety ceiling, not allocation promise */
#define QIHASH_MIN_PASSES 1
#define QIHASH_MAX_PASSES 64
#define QIHASH_ENCODED_MAX 2048

#define QIHASH_OK 0
#define QIHASH_ERR_NULL -1
#define QIHASH_ERR_PARAM -2
#define QIHASH_ERR_MEMORY -3
#define QIHASH_ERR_PARSE -4
#define QIHASH_ERR_VERIFY -5
#define QIHASH_ERR_RANDOM -6
#define QIHASH_ERR_IO -7

#define QIHASH_MODE_ID 1
#define QIHASH_MODE_SAFE 2
#define QIHASH_MODE_HARD 3

typedef struct qihash_params {
    uint32_t version;
    uint32_t memory_kib;
    uint32_t passes;
    uint32_t mode;
    uint32_t output_len;
} qihash_params;

typedef struct qihash_xof_ctx {
    uint64_t s[16];
    uint8_t buf[64];
    size_t pos;
    int finalized;
} qihash_xof_ctx;

const char *qihash_version_string(void);
const char *qihash_error_string(int code);

void qihash_params_default(qihash_params *params);
int qihash_params_profile(const char *profile_name, qihash_params *params);
int qihash_params_validate(const qihash_params *params);
const char *qihash_mode_name(uint32_t mode);
int qihash_mode_from_name(const char *name, uint32_t *mode);

void qihash_xof_init(qihash_xof_ctx *ctx, const char *domain);
void qihash_xof_absorb(qihash_xof_ctx *ctx, const void *data, size_t len);
void qihash_xof_absorb_u64(qihash_xof_ctx *ctx, uint64_t v);
void qihash_xof_final(qihash_xof_ctx *ctx);
void qihash_xof_squeeze(qihash_xof_ctx *ctx, uint8_t *out, size_t out_len);

int qihash_hash(const void *data, size_t len, uint8_t *out, size_t out_len);
int qihash_hash_file(const char *path, uint8_t *out, size_t out_len);

int qikdf(const uint8_t *password, size_t password_len,
          const uint8_t *salt, size_t salt_len,
          const uint8_t *pepper, size_t pepper_len,
          const uint8_t *device_secret, size_t device_secret_len,
          const qihash_params *params,
          uint8_t *out, size_t out_len);

int qihash_password_hash(const char *password,
                         const uint8_t *pepper, size_t pepper_len,
                         const uint8_t *device_secret, size_t device_secret_len,
                         const qihash_params *params,
                         char *encoded, size_t encoded_len);

int qihash_password_verify(const char *password,
                           const char *encoded,
                           const uint8_t *pepper, size_t pepper_len,
                           const uint8_t *device_secret, size_t device_secret_len);

int qihash_parse_encoded(const char *encoded,
                         qihash_params *params,
                         uint8_t *salt, size_t *salt_len,
                         uint8_t *hash, size_t *hash_len);

int qihash_random(uint8_t *out, size_t out_len);
int qihash_hex_encode(const uint8_t *in, size_t in_len, char *out, size_t out_len);
int qihash_hex_decode(const char *hex, uint8_t *out, size_t *out_len);
int qihash_b64url_encode(const uint8_t *in, size_t in_len, char *out, size_t out_len);
int qihash_b64url_decode(const char *in, uint8_t *out, size_t *out_len);
int qihash_constant_time_equal(const uint8_t *a, const uint8_t *b, size_t len);
void qihash_secure_zero(void *ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* QIHASH_H */
