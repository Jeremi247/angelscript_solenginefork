# What differs from upstream

This is an altered version of the AngelCode Scripting Library, marked as such as its licence requires ([LICENSE.md](LICENSE.md)). It is maintained for [SolEngine](https://github.com/Jeremi247/SolEngine) and is not the original. The original is [anjo76/angelscript](https://github.com/anjo76/angelscript).

| | |
|---|---|
| `master` | Upstream's main branch, unchanged — fast-forwarded, never committed to |
| `sol/main` | `master` plus the commits below, rebased onto `master` to take upstream's changes |
| Release tags | `sol-<upstream version>-<n>` on `sol/main` ([0001](docs/decisions/0001-the-fork-follows-upstream.md)) |

## Commits that change `sdk/`

Every commit on `sol/main` that changes a file under `sdk/`, oldest first, named by its subject, with the record that argues it. Commits touching only this file, `CLAUDE.md`, `check/` or `docs/` are not listed.

| Commit | Change | Record |
|---|---|---|
| A declaration reports where it was made | `AS_SOL_DECLARATION_POSITIONS`. Adds `angelscript_sol.h`, `as_sol.h` and `as_sol.cpp`, which every later feature uses | [0004](docs/decisions/0004-a-feature-is-tested-beside-upstreams-suite.md), [0005](docs/decisions/0005-a-declaration-reports-where-it-was-made.md) |

## Releases

| Tag | Upstream commit | SolEngine features |
|---|---|---|
| `sol-2.39.0-wip.1` | `c9e74722` (2026-10-05) | None. No commit of ours changes `sdk/` |
