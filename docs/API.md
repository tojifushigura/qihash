# qiHash C API

Include:

```c
#include "qihash.h"
```

## Hash data

```c
uint8_t out[64];
qihash_hash("hello", 5, out, sizeof(out));
```

## Hash file

```c
uint8_t out[64];
qihash_hash_file("file.bin", out, sizeof(out));
```

## Password hash

```c
qihash_params p;
qihash_params_profile("web", &p);

char encoded[QIHASH_ENCODED_MAX];
qihash_password_hash("secret", pepper, pepper_len, device, device_len, &p, encoded, sizeof(encoded));
```

## Verify password

```c
int r = qihash_password_verify("secret", encoded, pepper, pepper_len, device, device_len);
if (r == QIHASH_OK) {
    // valid
}
```

## Direct KDF

```c
qihash_params p;
qihash_params_profile("admin", &p);

uint8_t out[64];
qikdf(password, password_len, salt, salt_len, pepper, pepper_len, device, device_len, &p, out, sizeof(out));
```

## Profiles

```c
qihash_params_profile("test", &p);
qihash_params_profile("light", &p);
qihash_params_profile("web", &p);
qihash_params_profile("admin", &p);
qihash_params_profile("archive", &p);
```
