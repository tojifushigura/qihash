# qiHash v1 Specification

Status: v1.0.0 implementation specification.

This document describes the current qiHash family:

- `qiHash-XOF-v1`
- `qiKDF-v1`
- encoded password hash format

## Design goals

qiHash is designed around five practical goals:

1. No external runtime dependencies.
2. Portable C99 implementation.
3. Configurable memory-hard KDF.
4. Built-in support for salt, pepper and device-secret.
5. Versioned encoding for future algorithm upgrades.

## Non-goals

qiHash does not claim formal proof of memory-hardness in v1. It is not a NIST/FIPS standard and has not yet completed public cryptanalysis.

## Byte order

All integer serialization uses little-endian 64-bit words.

## qiHash-XOF-v1

`qiHash-XOF-v1` is a sponge-style extendable-output function with:

```text
state size: 1024 bits
rate:       512 bits
capacity:   512 bits
rounds:     24 ARX rounds per permutation
padding:    0x1f ... 0x80
```

The internal state consists of 16 little-endian 64-bit words.

### Permutation

The permutation is ARX-based:

```text
A = add
R = rotate
X = xor
```

Each round applies:

1. round constants injection;
2. four column G-mixes;
3. four diagonal G-mixes;
4. cross-word diffusion layer.

The implementation is in `src/qihash.c`.

## Field absorption

Variable-length fields are absorbed with labels and length prefixes:

```text
label || uint64_le(length) || data
```

This avoids ambiguous concatenation.

## qiKDF-v1 inputs

```text
password:       user secret
salt:           unique random salt, recommended 32 bytes
pepper:         optional server-side secret, not stored in DB
device_secret:  optional client/device secret, not stored in DB
memory_kib:     memory cost in KiB
passes:         number of memory passes
mode:           id/safe/hard
output_len:     16..128 bytes, recommended 64
```

## qiKDF-v1 phases

### 1. Seed phase

```text
seed = XOF(
  domain = "qiKDF-v1/init",
  version,
  memory_kib,
  passes,
  mode,
  output_len,
  field("password", password),
  field("salt", salt),
  field("pepper", pepper),
  field("device", device_secret)
)[0..63]
```

### 2. Memory allocation

The memory matrix is an array of 64-byte blocks:

```text
blocks = memory_kib * 1024 / 64
```

Each block stores eight 64-bit words.

### 3. Fill phase

Memory is filled serially. Each block depends on the previous block and the seed:

```text
M[i] = F(seed, i, M[i-1])
```

This prevents cheap precomputation and forces allocation of the memory matrix.

### 4. Mixing phase

For each pass and block, qiKDF chooses two reference blocks and applies the ARX block mixer:

```text
j = index1 mod blocks
k = index2 mod blocks
M[i] = Mix(M[i], M[j], M[k], pass, i)
```

Addressing depends on mode:

```text
safe:  data-independent indexes
hard:  data-dependent indexes
id:    pass 0 safe, later passes hard
```

### 5. Finalization phase

The final XOF absorbs:

```text
seed
blocks
memory_kib
passes
mode
all memory blocks
```

Then it squeezes `output_len` bytes.

## Encoded password format

```text
$qihash$v=1$mode=id$m=262144$t=4$out=64$s=<salt>$h=<hash>
```

`<salt>` and `<hash>` use base64url without padding.

## Versioning

All encoded hashes include `v=1`. Future incompatible changes must use another version.

## Recommended parameters

```text
test:    m=1024,    t=2, out=64
light:   m=65536,   t=3, out=64
web:     m=262144,  t=4, out=64
admin:   m=524288,  t=5, out=64
archive: m=1048576, t=6, out=64
```

## Constant-time verification

Verification compares derived output with stored output using constant-time equality.

## Randomness

Salt generation uses the operating system CSPRNG:

- `/dev/urandom` on Unix-like systems;
- CryptoAPI on Windows.
