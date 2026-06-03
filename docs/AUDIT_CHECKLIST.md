# Audit Checklist

## Code safety

- [ ] integer overflow review
- [ ] memory allocation limits
- [ ] parser robustness
- [ ] secure zeroization review
- [ ] Windows random source review
- [ ] POSIX random source review
- [ ] compiler optimization review for constant-time comparison

## Cryptographic review

- [ ] ARX permutation analysis
- [ ] sponge capacity review
- [ ] domain separation review
- [ ] padding review
- [ ] block mixer review
- [ ] indexing review
- [ ] time-memory tradeoff analysis
- [ ] GPU implementation benchmark
- [ ] ASIC/FPGA cost estimate
- [ ] side-channel review for `hard` and `id` modes

## Fuzzing

- [ ] encoded hash parser fuzzing
- [ ] CLI argument parser fuzzing
- [ ] base64url decoder fuzzing
- [ ] hex decoder fuzzing

## Interoperability

- [ ] test vectors for each mode
- [ ] test vectors for each profile
- [ ] big-endian platform test
- [ ] 32-bit platform test
