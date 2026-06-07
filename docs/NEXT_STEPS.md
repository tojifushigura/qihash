# qiHash Next Steps

This document tracks the practical next steps after the current public-review hardening pass.

## Immediate blockers before v0.1.0 release

1. Fix version string consistency:
   - `include/qihash.h` uses project version `0.1.0`.
   - `src/qihash.c` must return `qiHash 0.1.0` from `qihash_version_string()`.

2. Check GitHub Actions:
   - CI
   - Sanitizers
   - Portability

3. Run local release checklist:

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

4. Create GitHub release:
   - tag: `v0.1.0`
   - title: `qiHash v0.1.0 — First Public Review Release`

## v0.1.x work

- Expand deterministic test vectors.
- Add parser edge-case documentation.
- Add fuzz regression corpus folder.
- Publish first benchmark matrix.
- Update `SPEC.md` with clearer algorithm steps.
- Update `THREAT_MODEL.md` with limits and assumptions.

## v0.2.x work

- Stabilize CMake target names on all platforms.
- Add Windows build notes.
- Add macOS build notes.
- Add packaging notes.
- Improve allocation failure tests.

## v0.3.x work

- Run longer fuzzing sessions.
- Add crash/regression corpus workflow.
- Add more negative tests for codecs and parser.
- Add static-analysis workflow.

## v0.4.x work

- Publish benchmark data for several machines.
- Add JSON/CSV benchmark examples.
- Document recommended memory profiles.
- Document DoS risks for web deployments.

## v0.5.x work

- Freeze review specification.
- Publish whitepaper draft v0.2.
- Ask for public review.
- Contact review/audit organizations.
- Track all findings as GitHub issues.
