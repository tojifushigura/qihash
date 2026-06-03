# Competitor Analysis

This file summarizes the design space qiHash was built for.

## Argon2 / Argon2id

Argon2 is a memory-hard password hashing function and the winner of the Password Hashing Competition. RFC 9106 describes it as optimized for high memory filling rate and defense against trade-off attacks.

Design lesson for qiHash:

- memory hardness is mandatory;
- `id`-style hybrid addressing is practical;
- parameters must be encoded with the hash.

## scrypt

scrypt is a password-based KDF designed to make custom hardware attacks expensive through memory-hard computation.

Design lesson for qiHash:

- custom hardware pressure matters;
- memory cost must be explicit and tunable.

## yescrypt

yescrypt extends the scrypt design family and focuses on password hashing and hardware resistance.

Design lesson for qiHash:

- password hashing should be a full scheme, not only a digest primitive;
- multiple deployment profiles are useful.

## Balloon Hashing

Balloon Hashing emphasizes simple memory-hard construction and analysis of memory-hard properties.

Design lesson for qiHash:

- the algorithm should be easy enough to review;
- memory access and mixing phases should be documented clearly.

## Catena

Catena focuses on password-independent memory access patterns and side-channel considerations.

Design lesson for qiHash:

- a side-channel safer mode is needed;
- data-independent indexing has value for certain environments.

## qiHash differentiation

qiHash is not a clone of these projects. It uses its own C implementation, own ARX sponge/XOF core, own block mixer and own encoded format.

The architecture combines:

```text
qiHash-XOF-v1
+ qiKDF-v1 memory matrix
+ safe/hard/id modes
+ salt/pepper/device-secret API
+ versioned password hash format
```

## Honest limitation

The mature competitors have years of cryptanalysis. qiHash v1 has engineering completeness, but not the same review history yet. Public review and independent audit are required before making strong security claims.
