# qiHash Roadmap

This roadmap defines the next public milestones for qiHash.

## v0.1.x — Public baseline

- Keep the C99 implementation easy to build and review.
- Maintain GCC and Clang CI.
- Expand test vectors.
- Improve parser tests.
- Keep documentation clear for maintainers and contributors.

## v0.2.x — Portability

- Test Linux and macOS builds.
- Add Windows-compatible build notes.
- Improve install layout.
- Add optional CMake or Meson support if needed.
- Improve memory allocation failure handling.

## v0.3.x — Robustness testing

- Add fuzzing harnesses for encoded hash parsing.
- Add fuzzing for hex and base64url decoding.
- Add sanitizer CI jobs.
- Add more negative tests.
- Document edge cases.

## v0.4.x — Benchmarks

- Add machine-readable benchmark output.
- Collect CPU and memory scaling data.
- Document recommended profiles.
- Document web deployment limits.
- Compare parameter costs across hardware.

## v0.5.x — Review preparation

- Freeze a specification draft.
- Publish expanded test vectors.
- Update the threat model.
- Update the audit checklist.
- Collect public feedback through GitHub issues.

## Future stable release

A future stable release should be based on public review, fuzzing results, portability testing, benchmark data and external expert feedback.
