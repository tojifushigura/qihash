#include "qihash.h"

#include <stdio.h>
#include <string.h>

static int expect_parse_fail(const char *name, const char *encoded) {
    qihash_params params;
    uint8_t salt[128];
    uint8_t hash[256];
    size_t salt_len = sizeof(salt);
    size_t hash_len = sizeof(hash);
    int r = qihash_parse_encoded(encoded, &params, salt, &salt_len, hash, &hash_len);
    if (r == QIHASH_OK) {
        printf("%-36s FAIL\n", name);
        return 1;
    }
    printf("%-36s OK\n", name);
    return 0;
}

static int expect_parse_ok(const char *name, const char *encoded) {
    qihash_params params;
    uint8_t salt[128];
    uint8_t hash[256];
    size_t salt_len = sizeof(salt);
    size_t hash_len = sizeof(hash);
    int r = qihash_parse_encoded(encoded, &params, salt, &salt_len, hash, &hash_len);
    if (r != QIHASH_OK) {
        printf("%-36s FAIL code=%d\n", name, r);
        return 1;
    }
    printf("%-36s OK\n", name);
    return 0;
}

int main(void) {
    int fail = 0;

    fail |= expect_parse_fail("empty string", "");
    fail |= expect_parse_fail("wrong prefix", "$wrong$v=1$mode=id$m=1024$t=2$out=64$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("missing version", "$qihash$mode=id$m=1024$t=2$out=64$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("bad version", "$qihash$v=999$mode=id$m=1024$t=2$out=64$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("bad mode", "$qihash$v=1$mode=nope$m=1024$t=2$out=64$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("zero memory", "$qihash$v=1$mode=id$m=0$t=2$out=64$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("zero passes", "$qihash$v=1$mode=id$m=1024$t=0$out=64$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("short output", "$qihash$v=1$mode=id$m=1024$t=2$out=1$s=AAAA$h=BBBB");
    fail |= expect_parse_fail("missing salt", "$qihash$v=1$mode=id$m=1024$t=2$out=64$h=BBBB");
    fail |= expect_parse_fail("missing hash", "$qihash$v=1$mode=id$m=1024$t=2$out=64$s=AAAA");
    fail |= expect_parse_fail("bad base64", "$qihash$v=1$mode=id$m=1024$t=2$out=64$s=****$h=BBBB");
    fail |= expect_parse_fail("trailing field", "$qihash$v=1$mode=id$m=1024$t=2$out=64$s=AAAA$h=BBBB$x=1");

    fail |= expect_parse_ok("minimal valid structure", "$qihash$v=1$mode=id$m=1024$t=2$out=64$s=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA$h=BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");

    return fail ? 1 : 0;
}
