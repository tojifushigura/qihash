# Audit Outreach Guide

This guide helps maintainers contact reviewers, funders and audit teams.

## What qiHash needs first

qiHash should not start with an expensive full cryptographic audit immediately. The recommended order is:

1. Public code review.
2. Sanitizer and fuzzing results.
3. Benchmark matrix.
4. Specification cleanup.
5. External design review.
6. Professional implementation audit.
7. Deeper cryptographic review.

## Who to contact

### Open-source security organizations

- OSTIF — open-source security audits and review coordination.
- OpenSSF Alpha-Omega — open-source ecosystem security support.
- NLnet / NGI Zero — grants and support for open-source internet/security projects.

### Professional audit teams

- Cure53 — security assessments, code audits and cryptography audits.
- Trail of Bits — software assurance and security engineering.
- NCC Group Cryptography Services.
- Least Authority.
- Doyensec.

### Academic/community review

- Cryptography engineering communities.
- C security communities.
- Password hashing / KDF researchers.
- Open-source maintainers with C parser/fuzzing experience.

## Outreach email template

Subject:

```text
Review request for qiHash, an open-source C99 memory-hard KDF project
```

Body:

```text
Hello,

I maintain qiHash, an open-source C99 memory-hard hashing and KDF framework:
https://github.com/tojifushigura/qihash

The project is in its first public-review stage. I am looking for technical feedback on implementation robustness, parser safety, memory handling, KDF parameter validation, documentation quality and future review readiness.

Relevant files:
- README.md
- SPEC.md
- THREAT_MODEL.md
- SECURITY.md
- include/qihash.h
- src/qihash.c
- cli/qihash.c
- tests/
- fuzz/

Current work:
- CI build and tests
- sanitizer target
- fuzzing harnesses
- benchmark plan
- public issues and roadmap

I understand that new cryptographic designs require careful review before any production recommendation. At this stage, I am seeking feedback, review guidance and possible next steps toward a professional audit.

Would your team be open to reviewing the repository scope or advising what would be required for a formal review?

Thank you.
```

## What to attach or link

- Repository URL.
- Latest release URL.
- `docs/AUDIT_PLAN.md`.
- `docs/AUDIT_CHECKLIST.md`.
- `SPEC.md`.
- `THREAT_MODEL.md`.
- CI status.

## What not to claim

Avoid claims like:

- “proven secure”
- “quantum-proof”
- “better than Argon2id”
- “production-ready replacement”

Use safer language:

- “public-review release”
- “memory-hard KDF research project”
- “dependency-free C99 implementation”
- “seeking external review”
- “post-quantum search-cost assumptions”
