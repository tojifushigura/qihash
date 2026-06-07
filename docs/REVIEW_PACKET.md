# qiHash External Review Packet

This file is a quick entry point for people reviewing qiHash.

## Repository

https://github.com/tojifushigura/qihash

## Project summary

qiHash is a dependency-free C99 framework for data hashing and memory-hard password hashing / key derivation.

It includes:

- `qiHash-XOF-v1`
- `qiKDF-v1`
- CLI utility
- Static C library
- Password hash encoding and verification
- Salt, pepper and device-secret support
- Profiles and modes
- Tests
- Fuzz harnesses
- CI

## Build

```bash
make clean
make
make test
make example
./build/qihash selftest
```

## Sanitizers

```bash
make sanitize CC=clang
```

## Fuzz harnesses

```bash
make fuzz
./build/fuzz_parse_encoded -max_total_time=60
./build/fuzz_codecs -max_total_time=60
```

## Files to review first

- `include/qihash.h`
- `src/qihash.c`
- `cli/qihash.c`
- `tests/test_qihash.c`
- `fuzz/fuzz_parse_encoded.c`
- `fuzz/fuzz_codecs.c`
- `SPEC.md`
- `THREAT_MODEL.md`
- `SECURITY.md`

## Main review questions

1. Are all input lengths validated correctly?
2. Can encoded hash parsing be made safer or simpler?
3. Are allocation sizes bounded properly?
4. Are memory-cost parameters safe for real deployments?
5. Are failure modes documented clearly?
6. Are secrets cleared consistently where practical?
7. Are CLI inputs handled safely?
8. Are test vectors and docs sufficient for reproducibility?
9. Are there undefined-behavior risks in C operations?
10. What design assumptions need deeper analysis?

## Known limitations

- qiHash has not passed independent cryptographic review.
- The project is not recommended as a critical production replacement for reviewed schemes yet.
- Benchmark data is still incomplete.
- Fuzzing coverage is new and should be expanded.
- Cross-platform testing is incomplete.

## Desired review output

Please open GitHub issues for findings with:

- title;
- affected file/function;
- reproduction steps if applicable;
- severity estimate;
- suggested fix or mitigation;
- related tests or fuzz inputs.
