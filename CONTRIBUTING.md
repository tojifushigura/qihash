# Contributing to qiHash

Thank you for your interest in qiHash.

qiHash is a security-sensitive open-source project. Contributions are welcome, but changes must be reviewed carefully because mistakes in hashing, KDFs, parsing, memory handling or timing behavior can create real security risks.

## Project status

qiHash is a complete working implementation intended for public review, testing, benchmarking and hardening. It has not yet passed independent cryptographic audit.

Do not market qiHash as a proven replacement for audited password hashing algorithms until independent cryptanalysis and review are completed.

## How to contribute

1. Fork the repository.
2. Create a branch from `main`.
3. Make a focused change.
4. Run the full test suite.
5. Open a pull request with a clear description.

```bash
make clean
make
make test
make example
./build/qihash selftest
./build/qihash bench --profile test
```

## Good first contribution areas

- Documentation improvements.
- More test vectors.
- Build portability fixes.
- CI improvements.
- Benchmark reporting.
- Fuzzing harnesses.
- Static-analysis integration.

## Security-sensitive contribution areas

Please open an issue first before making large changes to:

- `qiHash-XOF-v1` internals.
- `qiKDF-v1` memory mixing.
- Password-hash parsing.
- Constant-time comparison.
- Salt, pepper or device-secret handling.
- Encoded hash format.
- Parameter validation.

## Coding style

- C99 only.
- No runtime dependencies.
- Keep code readable and portable.
- Prefer explicit bounds checks.
- Avoid undefined behavior.
- Avoid platform-specific assumptions unless guarded.
- Keep security-relevant parsing strict.

## Pull request checklist

Before opening a PR, confirm:

- [ ] `make clean && make` passes.
- [ ] `make test` passes.
- [ ] `make example` passes.
- [ ] New behavior is documented.
- [ ] Test vectors are updated when algorithm output changes.
- [ ] Security-sensitive changes are explained clearly.
- [ ] No generated `build/` artifacts are committed.

## Reporting vulnerabilities

Please do not open public issues for vulnerabilities. Follow the process in [`SECURITY.md`](SECURITY.md).
