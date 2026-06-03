# qiHash Threat Model

## Assets

qiHash protects:

- stored password hashes;
- derived keys from passwords;
- integrity digests for files/messages;
- server-side pepper separation;
- optional device-secret separation.

## Attacker models

### Offline database attacker

The attacker steals the application database and obtains:

```text
encoded password hash
salt
algorithm parameters
```

The attacker does not obtain:

```text
plaintext password
server pepper
device secret
MFA secret
```

qiKDF is designed to make each offline guess memory-expensive.

### GPU attacker

The attacker owns one or more GPUs. qiKDF makes each guess consume configurable RAM/VRAM and serial memory passes, reducing parallel throughput.

### ASIC/FPGA attacker

The attacker can build custom hardware. qiKDF tries to force high memory area/cost per guess via large memory matrix and random block references.

### Quantum search attacker

The attacker has future quantum search acceleration. qiHash uses 512-bit default output and recommends 256-bit salts/peppers/device secrets. This does not make weak passwords strong; it increases the margin for the cryptographic layer.

### Web DoS attacker

A public login endpoint using high memory cost can be abused. qiHash must be deployed with:

- rate limits;
- account/IP throttling;
- pre-KDF cheap abuse checks;
- queue limits;
- memory concurrency caps.

## Out of scope

qiHash does not protect against:

- malware on the user device;
- server compromise that exposes pepper;
- phishing;
- weak passwords without rate limiting;
- side-channel attacks in hostile shared hardware unless `safe` mode and additional isolation are used;
- cryptanalytic breaks discovered after release.
