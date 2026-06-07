# qiHash

[![CI](https://github.com/tojifushigura/qihash/actions/workflows/ci.yml/badge.svg)](https://github.com/tojifushigura/qihash/actions/workflows/ci.yml)
[![Sanitizers](https://github.com/tojifushigura/qihash/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/tojifushigura/qihash/actions/workflows/sanitizers.yml)
[![Portability](https://github.com/tojifushigura/qihash/actions/workflows/portability.yml/badge.svg)](https://github.com/tojifushigura/qihash/actions/workflows/portability.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Language: C99](https://img.shields.io/badge/language-C99-blue.svg)](include/qihash.h)
[![Security Policy](https://img.shields.io/badge/security-policy-important.svg)](SECURITY.md)

**qiHash** is an open-source C99 framework for data hashing and memory-hard password hashing / key derivation.

Full name:

```text
qiHash: Quantum Integrity Hashing Framework
```

Short description:

```text
Memory-hard hashing and KDF framework for post-quantum search-cost research.
```

## Status

qiHash is a complete working implementation intended for public review, testing, benchmarking and hardening.

Current release target:

```text
v0.1.0 — first public review release
```

Security status:

- Not independently cryptographically audited yet.
- Not recommended as a drop-in replacement for audited production password hashing schemes yet.
- Suitable for review, experimentation, benchmarking, education and further open-source hardening.

For production password storage today, use established and reviewed password hashing schemes. qiHash is a new independent design that must earn trust through review, testing, benchmarks and external analysis.

## Goals

qiHash aims to provide:

- A dependency-free C99 implementation.
- A clear memory-hard KDF design.
- Password hash encoding and verification.
- Salt, pepper and device-secret support.
- Profiles for different cost levels.
- Modes for different threat models: `safe`, `hard`, `id`.
- Simple CLI and static C library.
- Test vectors, self-test and benchmarks.
- Fuzzing, sanitizer and portability workflows.
- Documentation for public review and maintainers.

The target pressure model is:

```text
CPU + GPU + RAM/VRAM + ASIC/FPGA + quantum-search pressure
```

qiHash does not claim that hashes can be “decoded”. Hashes are not decrypted; passwords are guessed. qiKDF is designed to make each guess expensive in time and memory.

## Non-goals

qiHash does not aim to:

- Claim proven security without independent review.
- Replace Argon2id, scrypt or other reviewed schemes in critical systems today.
- Provide encryption, TLS, signatures or post-quantum key exchange.
- Hide weak user passwords without rate limits, MFA, pepper and deployment controls.
- Promise resistance against all future cryptanalytic discoveries.

## Components

- `qiHash-XOF-v1` — extendable-output hash core for data/file digests.
- `qiKDF-v1` — memory-hard password hashing / key derivation function.
- Password hash encoding and verification.
- Salt, pepper and device-secret support.
- CLI utility.
- Static C library.
- Tests, self-test, benchmark and C example.
- Fuzz harnesses and seed corpus.
- Specification, threat model and review packet.

## Build with Make

```bash
make
make test
```

The binary will be created here:

```bash
./build/qihash
```

## Build with CMake

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake --config Release
ctest --test-dir build-cmake --output-on-failure
```

## Install

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
HASH='$qihash$v=1$mode=id$m=1024$t=2$out=64$s=...$h=...'
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

Generate benchmark matrix:

```bash
sh scripts/benchmark_matrix.sh
```

## Fuzzing and sanitizers

Sanitizer test run:

```bash
make sanitize CC=clang
```

Build fuzz harnesses:

```bash
make fuzz
```

Run fuzzers:

```bash
./build/fuzz_parse_encoded fuzz/corpus/parse_encoded -max_total_time=60
./build/fuzz_codecs fuzz/corpus/codecs -max_total_time=60
```

## Password hash format

```text
$qihash$v=1$mode=id$m=262144$t=4$out=64$s=<base64url-salt>$h=<base64url-hash>
```

Fields:

| Field | Meaning |
|---|---|
| `v` | algorithm format version |
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

## Review and audit preparation

Start here:

- [`docs/REVIEW_PACKET.md`](docs/REVIEW_PACKET.md)
- [`docs/AUDIT_PLAN.md`](docs/AUDIT_PLAN.md)
- [`docs/AUDIT_OUTREACH.md`](docs/AUDIT_OUTREACH.md)
- [`docs/WHITEPAPER_DRAFT.md`](docs/WHITEPAPER_DRAFT.md)

## Repository layout

```text
qihash/
├── include/qihash.h
├── src/qihash.c
├── cli/qihash.c
├── tests/
├── fuzz/
├── scripts/
├── benchmarks/
├── examples/
├── docs/
├── .github/
├── CMakeLists.txt
├── SPEC.md
├── THREAT_MODEL.md
├── COMPETITOR_ANALYSIS.md
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── ROADMAP.md
├── SECURITY.md
├── Makefile
└── LICENSE
```

## How to contribute

Start with [`CONTRIBUTING.md`](CONTRIBUTING.md).

Useful contribution areas:

- Test vectors.
- Fuzzing harnesses.
- Static analysis.
- Portability fixes.
- Documentation.
- Benchmarks.
- Review of security assumptions.

For vulnerabilities, do not open a public issue. Follow [`SECURITY.md`](SECURITY.md).

## Roadmap

See [`ROADMAP.md`](ROADMAP.md).

Immediate public-review tasks:

- Independent cryptographic review.
- Fuzzing.
- Benchmarks.
- Documentation improvements.
- Portability testing.

## License

MIT License. See [`LICENSE`](LICENSE).
