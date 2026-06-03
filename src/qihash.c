#include "qihash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#endif

#define QI_RATE_BYTES 64
#define QI_BLOCK_WORDS 8
#define QI_STATE_WORDS 16

static uint64_t qi_rotl64(uint64_t x, unsigned n) {
    return (x << n) | (x >> (64 - n));
}

static uint64_t qi_load64_le(const uint8_t *p) {
    return ((uint64_t)p[0]) |
           ((uint64_t)p[1] << 8) |
           ((uint64_t)p[2] << 16) |
           ((uint64_t)p[3] << 24) |
           ((uint64_t)p[4] << 32) |
           ((uint64_t)p[5] << 40) |
           ((uint64_t)p[6] << 48) |
           ((uint64_t)p[7] << 56);
}

static void qi_store64_le(uint8_t *p, uint64_t v) {
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
    p[4] = (uint8_t)(v >> 32);
    p[5] = (uint8_t)(v >> 40);
    p[6] = (uint8_t)(v >> 48);
    p[7] = (uint8_t)(v >> 56);
}

static const uint64_t qi_iv[16] = {
    0x7169486173682d31ULL, 0x516875616e74756dULL,
    0x496e746772747931ULL, 0x4b44462d58304621ULL,
    0x9e3779b97f4a7c15ULL, 0xbf58476d1ce4e5b9ULL,
    0x94d049bb133111ebULL, 0xd6e8feb86659fd93ULL,
    0xa4093822299f31d0ULL, 0x082efa98ec4e6c89ULL,
    0x452821e638d01377ULL, 0xbe5466cf34e90c6cULL,
    0xc0ac29b7c97c50ddULL, 0x3f84d5b5b5470917ULL,
    0x9216d5d98979fb1bULL, 0xd1310ba698dfb5acULL
};

static const uint64_t qi_rc[24] = {
    0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL,
    0xa4093822299f31d0ULL, 0x082efa98ec4e6c89ULL,
    0x452821e638d01377ULL, 0xbe5466cf34e90c6cULL,
    0xc0ac29b7c97c50ddULL, 0x3f84d5b5b5470917ULL,
    0x9216d5d98979fb1bULL, 0xd1310ba698dfb5acULL,
    0x2ffd72dbd01adfb7ULL, 0xb8e1afed6a267e96ULL,
    0xba7c9045f12c7f99ULL, 0x24a19947b3916cf7ULL,
    0x0801f2e2858efc16ULL, 0x636920d871574e69ULL,
    0xa458fea3f4933d7eULL, 0x0d95748f728eb658ULL,
    0x718bcd5882154aeeULL, 0x7b54a41dc25a59b5ULL,
    0x9c30d5392af26013ULL, 0xc5d1b023286085f0ULL,
    0xca417918b8db38efULL, 0x8e79dcb0603a180eULL
};

static void qi_g(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
    *a += *b; *d ^= *a; *d = qi_rotl64(*d, 32);
    *c += *d; *b ^= *c; *b = qi_rotl64(*b, 24);
    *a += *b; *d ^= *a; *d = qi_rotl64(*d, 16);
    *c += *d; *b ^= *c; *b = qi_rotl64(*b, 63);
}

static void qi_permute(uint64_t s[16]) {
    for (unsigned r = 0; r < 24; r++) {
        s[r & 15] ^= qi_rc[r];
        s[(r + 5) & 15] += qi_rotl64(qi_rc[23 - r], (r % 63) + 1);

        qi_g(&s[0], &s[4], &s[8],  &s[12]);
        qi_g(&s[1], &s[5], &s[9],  &s[13]);
        qi_g(&s[2], &s[6], &s[10], &s[14]);
        qi_g(&s[3], &s[7], &s[11], &s[15]);

        qi_g(&s[0], &s[5], &s[10], &s[15]);
        qi_g(&s[1], &s[6], &s[11], &s[12]);
        qi_g(&s[2], &s[7], &s[8],  &s[13]);
        qi_g(&s[3], &s[4], &s[9],  &s[14]);

        for (unsigned i = 0; i < 16; i++) {
            s[i] ^= qi_rotl64(s[(i + 7) & 15] + qi_rc[(r + i) % 24], (unsigned)((i * 7 + r) % 63 + 1));
        }
    }
}

const char *qihash_version_string(void) {
    return "qiHash 1.0.0";
}

const char *qihash_error_string(int code) {
    switch (code) {
        case QIHASH_OK: return "ok";
        case QIHASH_ERR_NULL: return "null pointer";
        case QIHASH_ERR_PARAM: return "invalid parameter";
        case QIHASH_ERR_MEMORY: return "out of memory";
        case QIHASH_ERR_PARSE: return "parse error";
        case QIHASH_ERR_VERIFY: return "verification failed";
        case QIHASH_ERR_RANDOM: return "random source failed";
        case QIHASH_ERR_IO: return "I/O error";
        default: return "unknown error";
    }
}

void qihash_secure_zero(void *ptr, size_t len) {
    if (!ptr || len == 0) return;
#if defined(__STDC_LIB_EXT1__)
    memset_s(ptr, len, 0, len);
#else
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    while (len--) *p++ = 0;
#endif
}

int qihash_constant_time_equal(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t diff = 0;
    if (!a || !b) return 0;
    for (size_t i = 0; i < len; i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

void qihash_params_default(qihash_params *params) {
    if (!params) return;
    params->version = 1;
    params->memory_kib = 256 * 1024;
    params->passes = 4;
    params->mode = QIHASH_MODE_ID;
    params->output_len = QIHASH_HASH_BYTES_DEFAULT;
}

int qihash_params_profile(const char *profile_name, qihash_params *params) {
    if (!profile_name || !params) return QIHASH_ERR_NULL;
    qihash_params_default(params);
    if (strcmp(profile_name, "test") == 0) {
        params->memory_kib = 1024;
        params->passes = 2;
    } else if (strcmp(profile_name, "web") == 0 || strcmp(profile_name, "standard") == 0) {
        params->memory_kib = 256 * 1024;
        params->passes = 4;
    } else if (strcmp(profile_name, "admin") == 0 || strcmp(profile_name, "hard") == 0) {
        params->memory_kib = 512 * 1024;
        params->passes = 5;
    } else if (strcmp(profile_name, "archive") == 0 || strcmp(profile_name, "extreme") == 0) {
        params->memory_kib = 1024 * 1024;
        params->passes = 6;
    } else if (strcmp(profile_name, "light") == 0) {
        params->memory_kib = 64 * 1024;
        params->passes = 3;
    } else {
        return QIHASH_ERR_PARAM;
    }
    return QIHASH_OK;
}

int qihash_params_validate(const qihash_params *params) {
    if (!params) return QIHASH_ERR_NULL;
    if (params->version != 1) return QIHASH_ERR_PARAM;
    if (params->memory_kib < QIHASH_MIN_MEMORY_KIB || params->memory_kib > QIHASH_MAX_MEMORY_KIB) return QIHASH_ERR_PARAM;
    if (params->passes < QIHASH_MIN_PASSES || params->passes > QIHASH_MAX_PASSES) return QIHASH_ERR_PARAM;
    if (!(params->mode == QIHASH_MODE_ID || params->mode == QIHASH_MODE_SAFE || params->mode == QIHASH_MODE_HARD)) return QIHASH_ERR_PARAM;
    if (params->output_len < QIHASH_HASH_BYTES_MIN || params->output_len > QIHASH_HASH_BYTES_MAX) return QIHASH_ERR_PARAM;
    return QIHASH_OK;
}

const char *qihash_mode_name(uint32_t mode) {
    switch (mode) {
        case QIHASH_MODE_ID: return "id";
        case QIHASH_MODE_SAFE: return "safe";
        case QIHASH_MODE_HARD: return "hard";
        default: return "unknown";
    }
}

int qihash_mode_from_name(const char *name, uint32_t *mode) {
    if (!name || !mode) return QIHASH_ERR_NULL;
    if (strcmp(name, "id") == 0) *mode = QIHASH_MODE_ID;
    else if (strcmp(name, "safe") == 0) *mode = QIHASH_MODE_SAFE;
    else if (strcmp(name, "hard") == 0) *mode = QIHASH_MODE_HARD;
    else return QIHASH_ERR_PARAM;
    return QIHASH_OK;
}

static void qi_absorb_block(qihash_xof_ctx *ctx, const uint8_t block[QI_RATE_BYTES]) {
    for (size_t i = 0; i < QI_BLOCK_WORDS; i++) {
        ctx->s[i] ^= qi_load64_le(block + 8 * i);
    }
    qi_permute(ctx->s);
}

void qihash_xof_init(qihash_xof_ctx *ctx, const char *domain) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    for (size_t i = 0; i < QI_STATE_WORDS; i++) ctx->s[i] = qi_iv[i];
    ctx->s[0] ^= 0x5149484153482d58ULL;
    ctx->s[1] ^= (uint64_t)QI_RATE_BYTES;
    ctx->s[2] ^= 0x0001000000000000ULL;
    if (domain) {
        qihash_xof_absorb(ctx, domain, strlen(domain));
    }
    qihash_xof_absorb_u64(ctx, domain ? (uint64_t)strlen(domain) : 0);
}

void qihash_xof_absorb(qihash_xof_ctx *ctx, const void *data, size_t len) {
    if (!ctx || (!data && len)) return;
    const uint8_t *p = (const uint8_t *)data;
    while (len > 0) {
        size_t take = QI_RATE_BYTES - ctx->pos;
        if (take > len) take = len;
        memcpy(ctx->buf + ctx->pos, p, take);
        ctx->pos += take;
        p += take;
        len -= take;
        if (ctx->pos == QI_RATE_BYTES) {
            qi_absorb_block(ctx, ctx->buf);
            memset(ctx->buf, 0, sizeof(ctx->buf));
            ctx->pos = 0;
        }
    }
}

void qihash_xof_absorb_u64(qihash_xof_ctx *ctx, uint64_t v) {
    uint8_t b[8];
    qi_store64_le(b, v);
    qihash_xof_absorb(ctx, b, sizeof(b));
    qihash_secure_zero(b, sizeof(b));
}

void qihash_xof_final(qihash_xof_ctx *ctx) {
    if (!ctx || ctx->finalized) return;
    ctx->buf[ctx->pos] ^= 0x1f;
    ctx->buf[QI_RATE_BYTES - 1] ^= 0x80;
    qi_absorb_block(ctx, ctx->buf);
    memset(ctx->buf, 0, sizeof(ctx->buf));
    ctx->pos = 0;
    ctx->finalized = 1;
}

void qihash_xof_squeeze(qihash_xof_ctx *ctx, uint8_t *out, size_t out_len) {
    if (!ctx || (!out && out_len)) return;
    if (!ctx->finalized) qihash_xof_final(ctx);
    while (out_len > 0) {
        uint8_t block[QI_RATE_BYTES];
        for (size_t i = 0; i < QI_BLOCK_WORDS; i++) qi_store64_le(block + 8 * i, ctx->s[i]);
        size_t take = out_len < QI_RATE_BYTES ? out_len : QI_RATE_BYTES;
        memcpy(out, block, take);
        out += take;
        out_len -= take;
        qihash_secure_zero(block, sizeof(block));
        if (out_len > 0) {
            ctx->s[15] ^= 0x8000000000000000ULL;
            qi_permute(ctx->s);
        }
    }
}

static void qi_absorb_field(qihash_xof_ctx *ctx, const char *label, const uint8_t *data, size_t len) {
    qihash_xof_absorb(ctx, label, strlen(label));
    qihash_xof_absorb_u64(ctx, (uint64_t)len);
    if (data && len) qihash_xof_absorb(ctx, data, len);
}

int qihash_hash(const void *data, size_t len, uint8_t *out, size_t out_len) {
    if (!out || (!data && len)) return QIHASH_ERR_NULL;
    if (out_len < QIHASH_HASH_BYTES_MIN || out_len > QIHASH_HASH_BYTES_MAX) return QIHASH_ERR_PARAM;
    qihash_xof_ctx ctx;
    qihash_xof_init(&ctx, "qiHash-v1/hash");
    qi_absorb_field(&ctx, "data", (const uint8_t *)data, len);
    qihash_xof_squeeze(&ctx, out, out_len);
    qihash_secure_zero(&ctx, sizeof(ctx));
    return QIHASH_OK;
}

int qihash_hash_file(const char *path, uint8_t *out, size_t out_len) {
    if (!path || !out) return QIHASH_ERR_NULL;
    if (out_len < QIHASH_HASH_BYTES_MIN || out_len > QIHASH_HASH_BYTES_MAX) return QIHASH_ERR_PARAM;
    FILE *f = fopen(path, "rb");
    if (!f) return QIHASH_ERR_IO;
    qihash_xof_ctx ctx;
    uint8_t buf[8192];
    qihash_xof_init(&ctx, "qiHash-v1/file");
    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n) qihash_xof_absorb(&ctx, buf, n);
        if (n < sizeof(buf)) {
            if (ferror(f)) {
                fclose(f);
                qihash_secure_zero(&ctx, sizeof(ctx));
                qihash_secure_zero(buf, sizeof(buf));
                return QIHASH_ERR_IO;
            }
            break;
        }
    }
    fclose(f);
    qihash_xof_squeeze(&ctx, out, out_len);
    qihash_secure_zero(&ctx, sizeof(ctx));
    qihash_secure_zero(buf, sizeof(buf));
    return QIHASH_OK;
}

static void qi_block_from_xof(const char *domain, const uint8_t seed[64], uint64_t a, uint64_t b, uint64_t c, uint64_t out_words[8]) {
    uint8_t out[64];
    qihash_xof_ctx x;
    qihash_xof_init(&x, domain);
    qi_absorb_field(&x, "seed", seed, 64);
    qihash_xof_absorb_u64(&x, a);
    qihash_xof_absorb_u64(&x, b);
    qihash_xof_absorb_u64(&x, c);
    qihash_xof_squeeze(&x, out, sizeof(out));
    for (size_t i = 0; i < 8; i++) out_words[i] = qi_load64_le(out + i * 8);
    qihash_secure_zero(&x, sizeof(x));
    qihash_secure_zero(out, sizeof(out));
}

static void qi_mix_block(uint64_t dst[8], const uint64_t a[8], const uint64_t b[8], uint64_t pass, uint64_t index) {
    uint64_t x[16];
    for (size_t i = 0; i < 8; i++) {
        x[i] = dst[i] ^ qi_rotl64(a[i] + pass + 0x9e3779b97f4a7c15ULL * (i + 1), (unsigned)(i * 7 + 1));
        x[i + 8] = b[i] ^ qi_rotl64(dst[i] + index + 0xbf58476d1ce4e5b9ULL * (i + 1), (unsigned)(i * 9 + 3));
    }
    x[0] ^= pass;
    x[1] ^= index;
    qi_permute(x);
    for (size_t i = 0; i < 8; i++) {
        dst[i] = x[i] ^ x[i + 8] ^ qi_rotl64(dst[(i + 3) & 7], (unsigned)(17 + i));
    }
    qihash_secure_zero(x, sizeof(x));
}

static uint64_t qi_index_independent(const uint8_t seed[64], uint64_t pass, uint64_t i, uint64_t domain_tag) {
    uint64_t words[8];
    qi_block_from_xof("qiKDF-v1/index", seed, pass, i, domain_tag, words);
    uint64_t r = words[0] ^ qi_rotl64(words[3], 17) ^ words[7];
    qihash_secure_zero(words, sizeof(words));
    return r;
}

static uint64_t qi_index_dependent(const uint64_t prev[8], uint64_t pass, uint64_t i, uint64_t domain_tag) {
    uint64_t x = prev[0] ^ qi_rotl64(prev[2], 13) ^ qi_rotl64(prev[5], 29) ^ prev[7];
    x += 0x9e3779b97f4a7c15ULL * (pass + 1);
    x ^= 0xbf58476d1ce4e5b9ULL * (i + 1);
    x ^= 0x94d049bb133111ebULL * (domain_tag + 1);
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

int qikdf(const uint8_t *password, size_t password_len,
          const uint8_t *salt, size_t salt_len,
          const uint8_t *pepper, size_t pepper_len,
          const uint8_t *device_secret, size_t device_secret_len,
          const qihash_params *params,
          uint8_t *out, size_t out_len) {
    if ((!password && password_len) || (!salt && salt_len) || (!pepper && pepper_len) ||
        (!device_secret && device_secret_len) || !params || !out) return QIHASH_ERR_NULL;
    int v = qihash_params_validate(params);
    if (v != QIHASH_OK) return v;
    if (out_len != params->output_len) return QIHASH_ERR_PARAM;
    if (salt_len < 16) return QIHASH_ERR_PARAM;

    size_t total_bytes = (size_t)params->memory_kib * 1024u;
    size_t blocks = total_bytes / 64u;
    if (blocks < 8) return QIHASH_ERR_PARAM;
    if (blocks > (SIZE_MAX / sizeof(uint64_t) / 8u)) return QIHASH_ERR_PARAM;

    uint64_t *mem = (uint64_t *)calloc(blocks, 8 * sizeof(uint64_t));
    if (!mem) return QIHASH_ERR_MEMORY;

    uint8_t seed[64];
    qihash_xof_ctx x;
    qihash_xof_init(&x, "qiKDF-v1/init");
    qihash_xof_absorb_u64(&x, params->version);
    qihash_xof_absorb_u64(&x, params->memory_kib);
    qihash_xof_absorb_u64(&x, params->passes);
    qihash_xof_absorb_u64(&x, params->mode);
    qihash_xof_absorb_u64(&x, params->output_len);
    qi_absorb_field(&x, "password", password, password_len);
    qi_absorb_field(&x, "salt", salt, salt_len);
    qi_absorb_field(&x, "pepper", pepper, pepper_len);
    qi_absorb_field(&x, "device", device_secret, device_secret_len);
    qihash_xof_squeeze(&x, seed, sizeof(seed));
    qihash_secure_zero(&x, sizeof(x));

    uint64_t prev[8];
    qi_block_from_xof("qiKDF-v1/fill0", seed, 0, blocks, params->memory_kib, prev);
    for (size_t i = 0; i < blocks; i++) {
        uint64_t tmp[8];
        qi_block_from_xof("qiKDF-v1/fill", seed, (uint64_t)i, prev[0] ^ prev[7], (uint64_t)blocks, tmp);
        for (size_t w = 0; w < 8; w++) {
            mem[i * 8 + w] = tmp[w] ^ prev[w] ^ qi_rotl64((uint64_t)i + qi_rc[w], (unsigned)(w * 5 + 1));
        }
        memcpy(prev, mem + i * 8, sizeof(prev));
        qihash_secure_zero(tmp, sizeof(tmp));
    }

    for (uint32_t pass = 0; pass < params->passes; pass++) {
        for (size_t i = 0; i < blocks; i++) {
            uint64_t *cur = mem + i * 8;
            uint64_t *prevblk = mem + ((i == 0 ? blocks - 1 : i - 1) * 8);
            uint64_t r1, r2;
            int independent = (params->mode == QIHASH_MODE_SAFE) || (params->mode == QIHASH_MODE_ID && pass == 0);
            if (independent) {
                r1 = qi_index_independent(seed, pass, (uint64_t)i, 1);
                r2 = qi_index_independent(seed, pass, (uint64_t)i, 2);
            } else {
                r1 = qi_index_dependent(prevblk, pass, (uint64_t)i, 1);
                r2 = qi_index_dependent(cur, pass, (uint64_t)i, 2);
            }
            size_t j = (size_t)(r1 % blocks);
            size_t k = (size_t)((r2 ^ (uint64_t)i ^ ((uint64_t)j << 17)) % blocks);
            qi_mix_block(cur, mem + j * 8, mem + k * 8, pass, (uint64_t)i);
        }
    }

    qihash_xof_init(&x, "qiKDF-v1/final");
    qi_absorb_field(&x, "seed", seed, sizeof(seed));
    qihash_xof_absorb_u64(&x, blocks);
    qihash_xof_absorb_u64(&x, params->memory_kib);
    qihash_xof_absorb_u64(&x, params->passes);
    qihash_xof_absorb_u64(&x, params->mode);
    for (size_t i = 0; i < blocks; i++) {
        uint8_t b[64];
        for (size_t w = 0; w < 8; w++) qi_store64_le(b + w * 8, mem[i * 8 + w]);
        qihash_xof_absorb(&x, b, sizeof(b));
        qihash_secure_zero(b, sizeof(b));
    }
    qihash_xof_squeeze(&x, out, out_len);

    qihash_secure_zero(&x, sizeof(x));
    qihash_secure_zero(seed, sizeof(seed));
    qihash_secure_zero(prev, sizeof(prev));
    qihash_secure_zero(mem, blocks * 8 * sizeof(uint64_t));
    free(mem);
    return QIHASH_OK;
}

int qihash_random(uint8_t *out, size_t out_len) {
    if (!out && out_len) return QIHASH_ERR_NULL;
#if defined(_WIN32)
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) return QIHASH_ERR_RANDOM;
    BOOL ok = CryptGenRandom(hProv, (DWORD)out_len, out);
    CryptReleaseContext(hProv, 0);
    return ok ? QIHASH_OK : QIHASH_ERR_RANDOM;
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return QIHASH_ERR_RANDOM;
    size_t n = fread(out, 1, out_len, f);
    fclose(f);
    return n == out_len ? QIHASH_OK : QIHASH_ERR_RANDOM;
#endif
}

int qihash_hex_encode(const uint8_t *in, size_t in_len, char *out, size_t out_len) {
    static const char h[] = "0123456789abcdef";
    if ((!in && in_len) || !out) return QIHASH_ERR_NULL;
    if (out_len < in_len * 2 + 1) return QIHASH_ERR_PARAM;
    for (size_t i = 0; i < in_len; i++) {
        out[i * 2] = h[in[i] >> 4];
        out[i * 2 + 1] = h[in[i] & 15];
    }
    out[in_len * 2] = '\0';
    return QIHASH_OK;
}

static int qi_hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int qihash_hex_decode(const char *hex, uint8_t *out, size_t *out_len) {
    if (!hex || !out_len) return QIHASH_ERR_NULL;
    size_t len = strlen(hex);
    if (len % 2) return QIHASH_ERR_PARSE;
    size_t need = len / 2;
    if (!out || *out_len < need) { *out_len = need; return QIHASH_ERR_PARAM; }
    for (size_t i = 0; i < need; i++) {
        int hi = qi_hex_val(hex[i * 2]);
        int lo = qi_hex_val(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return QIHASH_ERR_PARSE;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    *out_len = need;
    return QIHASH_OK;
}

static const char qi_b64url[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int qihash_b64url_encode(const uint8_t *in, size_t in_len, char *out, size_t out_len) {
    if ((!in && in_len) || !out) return QIHASH_ERR_NULL;
    size_t need = ((in_len + 2) / 3) * 4;
    while (need && ((in_len % 3 == 1 && need >= 2) || (in_len % 3 == 2 && need >= 1))) {
        if (in_len % 3 == 1) { need -= 2; break; }
        if (in_len % 3 == 2) { need -= 1; break; }
    }
    if (out_len < need + 1) return QIHASH_ERR_PARAM;
    size_t o = 0, i = 0;
    while (i + 3 <= in_len) {
        uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8) | in[i+2];
        out[o++] = qi_b64url[(v >> 18) & 63];
        out[o++] = qi_b64url[(v >> 12) & 63];
        out[o++] = qi_b64url[(v >> 6) & 63];
        out[o++] = qi_b64url[v & 63];
        i += 3;
    }
    if (i < in_len) {
        uint32_t v = (uint32_t)in[i] << 16;
        out[o++] = qi_b64url[(v >> 18) & 63];
        if (i + 1 < in_len) {
            v |= (uint32_t)in[i+1] << 8;
            out[o++] = qi_b64url[(v >> 12) & 63];
            out[o++] = qi_b64url[(v >> 6) & 63];
        } else {
            out[o++] = qi_b64url[(v >> 12) & 63];
        }
    }
    out[o] = '\0';
    return QIHASH_OK;
}

static int qi_b64_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '-') return 62;
    if (c == '_') return 63;
    return -1;
}

int qihash_b64url_decode(const char *in, uint8_t *out, size_t *out_len) {
    if (!in || !out_len) return QIHASH_ERR_NULL;
    size_t len = strlen(in);
    if (len % 4 == 1) return QIHASH_ERR_PARSE;
    size_t need = (len / 4) * 3;
    if (len % 4 == 2) need += 1;
    else if (len % 4 == 3) need += 2;
    if (!out || *out_len < need) { *out_len = need; return QIHASH_ERR_PARAM; }
    size_t i = 0, o = 0;
    while (i + 4 <= len) {
        int a = qi_b64_val(in[i++]); int b = qi_b64_val(in[i++]);
        int c = qi_b64_val(in[i++]); int d = qi_b64_val(in[i++]);
        if (a < 0 || b < 0 || c < 0 || d < 0) return QIHASH_ERR_PARSE;
        uint32_t v = ((uint32_t)a << 18) | ((uint32_t)b << 12) | ((uint32_t)c << 6) | (uint32_t)d;
        out[o++] = (uint8_t)(v >> 16);
        out[o++] = (uint8_t)(v >> 8);
        out[o++] = (uint8_t)v;
    }
    if (i < len) {
        int a = qi_b64_val(in[i++]); int b = qi_b64_val(in[i++]);
        if (a < 0 || b < 0) return QIHASH_ERR_PARSE;
        uint32_t v = ((uint32_t)a << 18) | ((uint32_t)b << 12);
        out[o++] = (uint8_t)(v >> 16);
        if (i < len) {
            int c = qi_b64_val(in[i++]);
            if (c < 0) return QIHASH_ERR_PARSE;
            v |= (uint32_t)c << 6;
            out[o++] = (uint8_t)(v >> 8);
        }
    }
    *out_len = o;
    return QIHASH_OK;
}

int qihash_password_hash(const char *password,
                         const uint8_t *pepper, size_t pepper_len,
                         const uint8_t *device_secret, size_t device_secret_len,
                         const qihash_params *params_in,
                         char *encoded, size_t encoded_len) {
    if (!password || !encoded) return QIHASH_ERR_NULL;
    qihash_params params_local;
    const qihash_params *params = params_in;
    if (!params) { qihash_params_default(&params_local); params = &params_local; }
    int v = qihash_params_validate(params);
    if (v != QIHASH_OK) return v;
    uint8_t salt[QIHASH_SALT_BYTES_DEFAULT];
    uint8_t hash[QIHASH_HASH_BYTES_MAX];
    char salt_b64[128];
    char hash_b64[256];
    v = qihash_random(salt, sizeof(salt));
    if (v != QIHASH_OK) return v;
    v = qikdf((const uint8_t *)password, strlen(password), salt, sizeof(salt), pepper, pepper_len,
              device_secret, device_secret_len, params, hash, params->output_len);
    if (v != QIHASH_OK) { qihash_secure_zero(salt, sizeof(salt)); return v; }
    v = qihash_b64url_encode(salt, sizeof(salt), salt_b64, sizeof(salt_b64));
    if (v != QIHASH_OK) goto done;
    v = qihash_b64url_encode(hash, params->output_len, hash_b64, sizeof(hash_b64));
    if (v != QIHASH_OK) goto done;
    int n = snprintf(encoded, encoded_len, "$qihash$v=%u$mode=%s$m=%u$t=%u$out=%u$s=%s$h=%s",
                     params->version, qihash_mode_name(params->mode), params->memory_kib,
                     params->passes, params->output_len, salt_b64, hash_b64);
    if (n < 0 || (size_t)n >= encoded_len) v = QIHASH_ERR_PARAM;
    else v = QIHASH_OK;
done:
    qihash_secure_zero(salt, sizeof(salt));
    qihash_secure_zero(hash, sizeof(hash));
    qihash_secure_zero(salt_b64, sizeof(salt_b64));
    qihash_secure_zero(hash_b64, sizeof(hash_b64));
    return v;
}

static int qi_parse_u32(const char *s, uint32_t *out) {
    if (!s || !*s || !out) return QIHASH_ERR_PARSE;
    char *end = NULL;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno || !end || *end || v > 0xffffffffUL) return QIHASH_ERR_PARSE;
    *out = (uint32_t)v;
    return QIHASH_OK;
}

int qihash_parse_encoded(const char *encoded,
                         qihash_params *params,
                         uint8_t *salt, size_t *salt_len,
                         uint8_t *hash, size_t *hash_len) {
    if (!encoded || !params || !salt_len || !hash_len) return QIHASH_ERR_NULL;
    if (strncmp(encoded, "$qihash$", 8) != 0) return QIHASH_ERR_PARSE;
    char tmp[QIHASH_ENCODED_MAX];
    if (strlen(encoded) >= sizeof(tmp)) return QIHASH_ERR_PARSE;
    strcpy(tmp, encoded + 8);
    qihash_params p;
    memset(&p, 0, sizeof(p));
    char *save = NULL;
    char *tok = strtok_r(tmp, "$", &save);
    int got_s = 0, got_h = 0;
    while (tok) {
        char *eq = strchr(tok, '=');
        if (!eq) return QIHASH_ERR_PARSE;
        *eq = '\0';
        const char *key = tok;
        const char *val = eq + 1;
        if (strcmp(key, "v") == 0) {
            if (qi_parse_u32(val, &p.version) != QIHASH_OK) return QIHASH_ERR_PARSE;
        } else if (strcmp(key, "mode") == 0) {
            if (qihash_mode_from_name(val, &p.mode) != QIHASH_OK) return QIHASH_ERR_PARSE;
        } else if (strcmp(key, "m") == 0) {
            if (qi_parse_u32(val, &p.memory_kib) != QIHASH_OK) return QIHASH_ERR_PARSE;
        } else if (strcmp(key, "t") == 0) {
            if (qi_parse_u32(val, &p.passes) != QIHASH_OK) return QIHASH_ERR_PARSE;
        } else if (strcmp(key, "out") == 0) {
            if (qi_parse_u32(val, &p.output_len) != QIHASH_OK) return QIHASH_ERR_PARSE;
        } else if (strcmp(key, "s") == 0) {
            size_t cap = *salt_len;
            int r = qihash_b64url_decode(val, salt, &cap);
            if (r != QIHASH_OK) return r;
            *salt_len = cap;
            got_s = 1;
        } else if (strcmp(key, "h") == 0) {
            size_t cap = *hash_len;
            int r = qihash_b64url_decode(val, hash, &cap);
            if (r != QIHASH_OK) return r;
            *hash_len = cap;
            got_h = 1;
        } else {
            return QIHASH_ERR_PARSE;
        }
        tok = strtok_r(NULL, "$", &save);
    }
    if (!got_s || !got_h) return QIHASH_ERR_PARSE;
    int r = qihash_params_validate(&p);
    if (r != QIHASH_OK) return r;
    if (*hash_len != p.output_len) return QIHASH_ERR_PARSE;
    *params = p;
    qihash_secure_zero(tmp, sizeof(tmp));
    return QIHASH_OK;
}

int qihash_password_verify(const char *password,
                           const char *encoded,
                           const uint8_t *pepper, size_t pepper_len,
                           const uint8_t *device_secret, size_t device_secret_len) {
    if (!password || !encoded) return QIHASH_ERR_NULL;
    qihash_params params;
    uint8_t salt[128]; size_t salt_len = sizeof(salt);
    uint8_t expected[QIHASH_HASH_BYTES_MAX]; size_t expected_len = sizeof(expected);
    int r = qihash_parse_encoded(encoded, &params, salt, &salt_len, expected, &expected_len);
    if (r != QIHASH_OK) return r;
    uint8_t actual[QIHASH_HASH_BYTES_MAX];
    r = qikdf((const uint8_t *)password, strlen(password), salt, salt_len, pepper, pepper_len,
              device_secret, device_secret_len, &params, actual, params.output_len);
    if (r != QIHASH_OK) goto done;
    r = qihash_constant_time_equal(expected, actual, expected_len) ? QIHASH_OK : QIHASH_ERR_VERIFY;
done:
    qihash_secure_zero(salt, sizeof(salt));
    qihash_secure_zero(expected, sizeof(expected));
    qihash_secure_zero(actual, sizeof(actual));
    return r;
}
