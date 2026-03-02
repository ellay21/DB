# Contributing

Thank you for your interest in Revenant.

## Quick start

```bash
git clone https://github.com/you/revenant && cd revenant
./scripts/bootstrap.sh
```

## Gate ladder

| Gate | Trigger | Budget | Contents |
|------|---------|--------|----------|
| G1 pre-commit | `git commit` | < 5 s | clang-format, cmake-format, trailing whitespace, codespell, shellcheck, commit-msg lint |
| G2 pre-push | `git push` | < 120 s | configure + build (ccache), `ctest -L unit`, header self-containment check |
| G3 PR CI | PR opened/updated | < 20 min | Full matrix build, unit + property + sanitizers, packaging, ABI diff |
| G4 Nightly | schedule | hours | Chaos, soak, fuzz, formal model checking |

**Do not use `--no-verify` on a branch with an open PR.** CI is the backstop.

## Commit convention

[Conventional Commits](https://www.conventionalcommits.org/) enforced by `.githooks/commit-msg`:

```
<type>(<scope>): <subject>

[body]

[footer]
```

**Types:** `feat`, `fix`, `test`, `perf`, `refactor`, `docs`, `build`, `ci`, `chore`, `spec`, `bench`.

**Scopes:** `core`, `ring`, `frame`, `claim`, `producer`, `consumer`, `flow`, `liveness`, `reaper`, `failover`, `platform`, `sim`, `channel`, `capi`, `ctl`, `diag`, `spec`, `build`, `ci`, `docs`.

**TDD marker** — test-first commits carry a `RED` marker; the implementation carries `GREEN`:

```
test(ring): RED wrap handling emits padding at capacity boundary
feat(ring): GREEN padding frame emission on wrap
```

**Required footers for protocol changes:**

```
Invariants: I3, I9
Wire-format: unchanged
```

## Definition of done

- [ ] Tests written before implementation (RED/GREEN visible in commit order)
- [ ] All new public API documented with Doxygen comments including ordering guarantees
- [ ] `clang-format` clean, `clang-tidy` clean
- [ ] No new compiler warnings on any matrix entry (warnings are errors)
- [ ] Invariant IDs listed in commit footer and covered by a test
- [ ] If layout changed: `wire-format.md` updated, `Wire-format:` footer set
- [ ] If a design decision was made: an ADR exists
- [ ] `CHANGELOG.md` updated under `## [Unreleased]`
- [ ] CI green

## Coverage policy

- **Project coverage**: reported, never enforced.
- **Patch coverage on `include/revenant/core/**` and `include/revenant/liveness/**`**: 90%, enforced.
