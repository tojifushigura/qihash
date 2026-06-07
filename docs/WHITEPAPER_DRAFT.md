# qiHash Whitepaper Draft

Version: draft-0.1

Repository: https://github.com/tojifushigura/qihash

## Abstract

qiHash is an open-source C99 framework for data hashing and memory-hard password hashing / key derivation. The project explores a dependency-free implementation with configurable memory cost, pass count, encoded password hashes, salt, pepper, device-secret support and deployment profiles.

This document is a draft for public review. It is not a security proof and should not be treated as an audit report.

## Motivation

Password hashing must resist large-scale guessing. Fast hashes are not suitable for password storage because they allow cheap repeated guesses. qiHash focuses on increasing the cost of each guess through memory use, pass count and deployment controls.

The target pressure model is:

```text
CPU + GPU + RAM/VRAM + ASIC/FPGA + quantum-search pressure
```

## Design goals

- Dependency-free C99 codebase.
- Public review friendly implementation.
- Memory-hard KDF mode.
- Encoded password hash format.
- Salt support.
- Optional server-side pepper.
- Optional device-secret binding.
- Tunable memory and time cost.
- Profiles for test, light, web, admin and archive use.
- CLI and library API.
- CI, tests, fuzzing and benchmark tooling.

## Non-goals

- qiHash does not claim proven cryptographic security without review.
- qiHash is not positioned as a production replacement for audited password hashing schemes at this stage.
- qiHash does not provide encryption, key exchange, signatures or TLS.
- qiHash does not make weak passwords safe without deployment controls.

## Components

### qiHash-XOF-v1

An extendable-output core used by the implementation for hashing and state expansion.

### qiKDF-v1

A memory-hard password hashing / key derivation function with configurable parameters.

### Encoded password format

```text
$qihash$v=1$mode=id$m=262144$t=4$out=64$s=<salt>$h=<hash>
```

Pepper and device-secret are intentionally not stored in the encoded string.

## Profiles

| Profile | Memory | Passes | Use case |
|---|---:|---:|---|
| test | 1 MiB | 2 | CI and local tests |
| light | 64 MiB | 3 | constrained systems |
| web | 256 MiB | 4 | normal login |
| admin | 512 MiB | 5 | privileged accounts |
| archive | 1 GiB | 6 | local archive/master-key use |

## Modes

| Mode | Description |
|---|---|
| safe | data-independent addressing preference |
| hard | data-dependent addressing preference |
| id | first pass safe, later passes hard |

## Threat model assumptions

qiHash assumes the attacker may obtain encoded password hashes and attempt offline guessing. The design attempts to make each guess costly in memory and time.

qiHash does not assume:

- strong user passwords by default;
- unlimited server resources;
- protection from endpoint compromise;
- protection from every side-channel;
- cryptographic proof at this stage.

## Deployment requirements

A real deployment should combine qiHash with:

- unique random salts;
- pepper stored outside the database;
- MFA for privileged users;
- rate limits;
- login abuse protection;
- parameter upgrade and rehash policy;
- monitoring and incident response.

## Review questions

1. Is the memory-hard construction clear enough for review?
2. Are parameter bounds appropriate?
3. Is parser behavior strict enough?
4. Are allocation checks sufficient?
5. Are there undefined-behavior risks?
6. Are the profiles practical?
7. What parts need formal analysis?
8. What benchmark data is needed?
9. What test vectors are missing?
10. What should be changed before a stable release?

## Current status

qiHash is in first public-review stage. It includes implementation, CLI, tests, CI, sanitizer workflow, fuzz harnesses, documentation and review planning. More benchmark data and external feedback are required before any strong deployment recommendation.
