# qiHash Audit and Review Plan

This document describes how qiHash should move from a first public release toward stronger external review.

## Current status

- Project: qiHash
- Repository: https://github.com/tojifushigura/qihash
- Language: C99
- Current release target: v0.1.0
- Encoded format version: v=1
- Status: public review and hardening

## Review goals

1. Confirm implementation robustness.
2. Review memory handling and parser behavior.
3. Review CLI input handling.
4. Review KDF parameter validation.
5. Review constant-time verification logic.
6. Review test vectors and reproducibility.
7. Review documentation and deployment guidance.
8. Identify gaps before any production recommendation.

## Scope

Primary files:

- `include/qihash.h`
- `src/qihash.c`
- `cli/qihash.c`
- `tests/test_qihash.c`
- `fuzz/`
- `SPEC.md`
- `THREAT_MODEL.md`
- `SECURITY.md`

## Stage 1: maintainer hardening

- [ ] Keep CI green.
- [ ] Add sanitizer builds.
- [ ] Add fuzzing harnesses.
- [ ] Add negative tests for parser failures.
- [ ] Add test-vector regeneration notes.
- [ ] Add benchmark matrix.
- [ ] Review all allocation and length checks.

## Stage 2: public review

- [ ] Create a review-focused GitHub issue.
- [ ] Publish a short design summary.
- [ ] Ask C/security contributors to review implementation safety.
- [ ] Ask cryptography reviewers to review design assumptions.
- [ ] Track comments as issues.

## Stage 3: external professional review

Prepare a review packet containing:

- Project summary.
- Threat model.
- Specification.
- Build and test commands.
- Critical files.
- Known limitations.
- Requested review scope.
- Budget/funding status.

## Review packet checklist

- [ ] `README.md` is up to date.
- [ ] `SPEC.md` is up to date.
- [ ] `THREAT_MODEL.md` is up to date.
- [ ] `SECURITY.md` is up to date.
- [ ] `docs/AUDIT_CHECKLIST.md` is up to date.
- [ ] `docs/AUDIT_OUTREACH.md` exists.
- [ ] CI is passing.
- [ ] Fuzz harnesses build.
- [ ] Latest release tag exists.

## Expected outputs

A useful review should produce:

- Findings list.
- Severity classification.
- Reproduction steps.
- Suggested fixes.
- Open design questions.
- Follow-up review recommendations.
