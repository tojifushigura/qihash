# qiHash Roadmap

This roadmap defines the next public milestones for qiHash.

## v0.1.x — Public baseline

- [x] Keep the C99 implementation easy to build and review.
- [x] Maintain GCC and Clang CI.
- [x] Add issue templates and PR template.
- [x] Add sanitizer workflow.
- [x] Add initial fuzzing harnesses.
- [x] Add negative parser tests.
- [x] Add CMake build support.
- [x] Add portability CI for Linux, macOS and Windows.
- [x] Add benchmark matrix script.
- [x] Add initial review/audit documentation.
- [ ] Expand test vectors.
- [ ] Confirm release notes for `v0.1.0`.

## v0.2.x — Portability

- [ ] Stabilize CMake output names across platforms.
- [ ] Add Windows-compatible build notes.
- [ ] Improve install layout.
- [ ] Add package-friendly release archives.
- [ ] Improve memory allocation failure handling.
- [ ] Test GCC, Clang, AppleClang and MSVC.

## v0.3.x — Robustness testing

- [x] Add fuzzing harnesses for encoded hash parsing.
- [x] Add fuzzing for hex and base64url decoding.
- [x] Add sanitizer CI jobs.
- [x] Add more negative tests.
- [ ] Add seed corpus expansion.
- [ ] Add regression corpus folder.
- [ ] Document parser edge cases.
- [ ] Run longer local fuzzing sessions.

## v0.4.x — Benchmarks

- [x] Add machine-readable benchmark output script.
- [ ] Collect CPU and memory scaling data.
- [ ] Publish initial benchmark matrix.
- [ ] Document recommended profiles.
- [ ] Document web deployment limits.
- [ ] Compare parameter costs across hardware.

## v0.5.x — Review preparation

- [ ] Freeze a specification draft.
- [ ] Publish expanded test vectors.
- [ ] Update the threat model.
- [ ] Update the audit checklist.
- [x] Add audit/review plan.
- [x] Add external review packet.
- [x] Add outreach template.
- [ ] Collect public feedback through GitHub issues.

## Future stable release

A future stable release should be based on public review, fuzzing results, portability testing, benchmark data and external expert feedback.
