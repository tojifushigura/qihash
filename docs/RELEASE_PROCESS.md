# Release Process

This document describes how to publish qiHash releases.

## Versioning

qiHash uses two separate version concepts:

1. Project release version, for example `v0.1.0`.
2. Encoded algorithm format version, for example `$qihash$v=1$...`.

The encoded format version may remain `v=1` while the project release version changes.

## Pre-release checklist

Before creating a release:

```bash
make clean
make
make test
make example
./build/qihash selftest
make sanitize CC=clang
make fuzz
sh scripts/benchmark_matrix.sh
```

Also check:

- [ ] CI workflow is passing.
- [ ] Sanitizers workflow is passing.
- [ ] Portability workflow is passing.
- [ ] `README.md` is current.
- [ ] `SECURITY.md` supported versions are current.
- [ ] `ROADMAP.md` is current.
- [ ] `docs/REVIEW_PACKET.md` is current.
- [ ] Release notes are clear about security status.

## Create a GitHub release

1. Open GitHub Releases.
2. Click **Draft a new release**.
3. Create a new tag such as `v0.1.0`.
4. Target branch: `main`.
5. Add release title and notes.

## Recommended title

```text
qiHash v0.1.0 — First Public Review Release
```

## Recommended notes

```markdown
# qiHash v0.1.0 — First Public Review Release

This is the first public-review release of qiHash.

## Included

- C99 static library
- CLI utility
- qiHash-XOF-v1
- qiKDF-v1
- Password hash encoding and verification
- Salt, pepper and device-secret support
- Profiles: test, light, web, admin, archive
- Modes: safe, hard, id
- Tests and negative parser tests
- CMake support
- CI, sanitizer workflow and portability workflow
- Fuzzing harnesses and initial seed corpus
- Benchmark matrix script
- Review packet and audit planning docs

## Security status

qiHash has not passed independent cryptographic review yet. This release is intended for public review, testing, benchmarking and hardening. Do not use it as a drop-in replacement for established password hashing schemes in critical production systems.
```

## After release

- Announce the release.
- Ask for public review.
- Run benchmark matrix locally.
- Attach benchmark results or publish them in `benchmarks/results/`.
- Track feedback as GitHub issues.
