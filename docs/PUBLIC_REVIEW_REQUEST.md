# Public Review Request

Use this text when asking developers, maintainers or security communities to review qiHash.

## Short version

```text
I am looking for public technical review of qiHash, an open-source C99 memory-hard hashing and KDF framework.

Repository:
https://github.com/tojifushigura/qihash

Current status: v0.1.0 public-review stage.

I am especially looking for feedback on:
- C memory handling;
- parser strictness;
- undefined-behavior risks;
- portability;
- tests and fuzzing;
- benchmark methodology;
- documentation clarity;
- design assumptions that need deeper analysis.

qiHash is not presented as audited production cryptography yet. The goal is to improve the project through open review before seeking a deeper audit.
```

## Longer version

```text
Hello,

I maintain qiHash, an open-source C99 memory-hard hashing and KDF framework:
https://github.com/tojifushigura/qihash

The project includes a CLI, static library, password-hash encoding/verification, salt, pepper and device-secret support, configurable memory cost, tests, fuzzing harnesses, CI, sanitizer workflow, CMake support and review documentation.

I am looking for public technical feedback before moving toward external audit. Areas where review would be especially useful:

1. C memory handling and allocation checks.
2. Encoded hash parser strictness.
3. Hex/base64url decoder edge cases.
4. Undefined-behavior risks.
5. Portability across Linux, macOS and Windows.
6. Fuzzing coverage.
7. Benchmark methodology.
8. Documentation clarity.
9. Design assumptions that need deeper cryptographic analysis.

Important: qiHash is not marketed as proven or audited production cryptography. It is a public-review release intended for testing, critique and hardening.

Feedback through GitHub issues is welcome.
```
