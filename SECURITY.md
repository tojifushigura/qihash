# Security Policy

## Supported version

| Version | Status |
|---|---|
| 0.1.x | Supported for public review and testing |

## Reporting vulnerabilities

Open a private security advisory or contact the maintainer privately before public disclosure.

Please include:

- affected version;
- reproduction steps;
- expected vs actual behavior;
- security impact;
- suggested fix if available.

## Deployment warning

qiHash is a complete implementation, but it is a new cryptographic design. New cryptography requires independent review before high-risk deployment.

For critical production systems today, use audited password hashing schemes such as Argon2id unless qiHash has passed the level of review required by your risk model.

## Safe deployment checklist

- Use unique random 32-byte salts.
- Use a server-side pepper stored outside the database.
- Use `id` mode by default.
- Use memory profiles appropriate to your hardware.
- Protect login endpoints against DoS.
- Use MFA for privileged users.
- Rehash old hashes when parameters are upgraded.
- Keep algorithm version in the encoded hash.
