# Deployment Guide

## Recommended architecture

```text
application database:
  encoded qiHash string
  user id
  password version

secret storage:
  pepper

optional client storage:
  device secret
```

## Do not store pepper in the same database

Pepper should be stored separately:

- environment secret as a minimum;
- Vault/KMS/HSM preferably;
- different access policy from the database.

## Web login protection

Before calling qiKDF on a public endpoint, enforce:

- per-IP rate limit;
- per-account rate limit;
- exponential backoff;
- bot protection;
- max concurrent KDF workers;
- memory budget cap.

## Suggested starting profiles

For normal web applications:

```text
profile: web
mode: id
memory: 256 MiB
passes: 4
output: 64 bytes
```

For admin users:

```text
profile: admin
mode: id
memory: 512 MiB
passes: 5
output: 64 bytes
MFA: required
```

For local archive encryption:

```text
profile: archive
mode: id/hard
memory: 1 GiB or more
passes: 6+
output: 64 or 128 bytes
```

## Rehash policy

When parameters change, keep old hashes verifiable. On successful login:

1. parse encoded hash;
2. compare old parameters with current policy;
3. if weaker, create a new encoded hash;
4. replace the stored hash.
