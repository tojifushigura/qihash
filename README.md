# qiHash

**qiHash** — open-source C99 framework for data hashing and memory-hard password hashing/KDF.

Full name:

```text
qiHash: Quantum Integrity Hashing Framework
```

The project is written from scratch, has no runtime dependencies, and includes:

- `qiHash-XOF-v1` — extendable-output hash core for data/file digests.
- `qiKDF-v1` — memory-hard password hashing / key derivation function.
- Password hash encoding and verification.
- Salt, pepper and device-secret support.
- Modes for different threat models: `safe`, `hard`, `id`.
- CLI utility.
- Static C library.
- Tests, self-test, benchmark and C example.
- Specification and threat model.

> Important: qiHash is a complete working implementation, not a demo. However, any new cryptographic algorithm still requires external cryptanalysis and audit before being trusted for high-risk production systems. This repository is engineered for open review.

## Why qiHash exists

Classical fast hashes such as MD5/SHA-1/SHA-256 are not suitable for password storage because attackers can compute huge numbers of guesses very quickly. qiHash focuses on making each password guess expensive in time and memory.

The target pressure model is:

```text
CPU + GPU + RAM/VRAM + ASIC/FPGA + quantum-search pressure
```

qiHash does not claim that hashes can be “decoded”. Hashes are not decrypted; passwords are guessed. qiKDF-v1 is designed to make each guess expensive.

## Build

```bash
make
make test
```

The binary will be created here:

```bash
./build/qihash
```

Install:

```bash
make install PREFIX=/usr/local
```

Uninstall:

```bash
make uninstall PREFIX=/usr/local
```

## CLI examples

Hash text:

```bash
./build/qihash hash hello
```

Hash file:

```bash
./build/qihash hash --file ./README.md --out 64
```

Create password hash:

```bash
./build/qihash password-hash --password 'secret' --profile web
```

For a quick local test profile:

```bash
./build/qihash password-hash --password 'secret' --profile test
```

Verify password hash:

```bash
HASH='./...'
./build/qihash password-verify --password 'secret' --encoded "$HASH"
```

Use pepper:

```bash
./build/qihash password-hash \
  --password 'secret' \
  --profile web \
  --pepper 00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff
```

Run benchmark:

```bash
./build/qihash bench --profile test
./build/qihash bench --profile light
```

## Password hash format

```text
$qihash$v=1$mode=id$m=262144$t=4$out=64$s=<base64url-salt>$h=<base64url-hash>
```

Fields:

| Field | Meaning |
|---|---|
| `v` | algorithm version |
| `mode` | `id`, `safe`, or `hard` |
| `m` | memory cost in KiB |
| `t` | number of passes |
| `out` | output length in bytes |
| `s` | random salt, base64url |
| `h` | derived hash, base64url |

Pepper and device-secret are **not** stored in the encoded string.

## Profiles

| Profile | Memory | Passes | Use case |
|---|---:|---:|---|
| `test` | 1 MiB | 2 | CI/tests only |
| `light` | 64 MiB | 3 | weak servers |
| `web` | 256 MiB | 4 | normal web login |
| `admin` | 512 MiB | 5 | admin/root accounts |
| `archive` | 1 GiB | 6 | local archives/master keys |

Tune values to your hardware. For public web logins, combine qiKDF with rate limiting and abuse protection.

## Modes

| Mode | Meaning |
|---|---|
| `safe` | data-independent addressing, better against cache/timing leakage |
| `hard` | data-dependent addressing, stronger pressure against GPU/ASIC parallelism |
| `id` | first pass safe, later passes hard; default balanced mode |

## Library usage

See [`examples/c_example.c`](examples/c_example.c).

```c
qihash_params params;
qihash_params_profile("web", &params);

char encoded[QIHASH_ENCODED_MAX];
qihash_password_hash("secret", pepper, pepper_len, NULL, 0, &params, encoded, sizeof(encoded));

int ok = qihash_password_verify("secret", encoded, pepper, pepper_len, NULL, 0);
```

## Repository layout

```text
qihash/
├── include/qihash.h
├── src/qihash.c
├── cli/qihash.c
├── tests/test_qihash.c
├── examples/c_example.c
├── docs/
│   ├── API.md
│   ├── CLI.md
│   ├── DEPLOYMENT.md
│   └── AUDIT_CHECKLIST.md
├── SPEC.md
├── THREAT_MODEL.md
├── COMPETITOR_ANALYSIS.md
├── SECURITY.md
├── Makefile
└── LICENSE
```

## Security status

qiHash is released for open review and controlled experimentation. Do not replace audited production password hashing with qiHash until the algorithm has received independent cryptographic analysis.

For production today, standard algorithms such as Argon2id remain the safe recommendation. qiHash is a new independent design intended for review, benchmarking and future hardening.
