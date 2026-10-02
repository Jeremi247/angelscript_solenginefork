# 0003 — A commit of ours keeps upstream's shape

Scope: what a commit on `sol/main` may touch, and how it is written. What it is for is [0002](0002-an-engine-feature-is-off-by-default.md)'s, and where it goes is [0001](0001-the-fork-follows-upstream.md)'s.

## The decision

**Code under `sdk/` is written in upstream's style**: its naming, its formatting, its comment voice and its C++ subset. SolEngine's [C++ style](https://github.com/Jeremi247/SolEngine/blob/main/docs/conventions/cpp-style.md) does not apply, and nothing here includes a SolEngine header.

**A new file is preferred to an edit, and a call into it to an edit spread across a function.** Where a feature can live in a file of its own with one guarded call from upstream's code, it does.

**Nothing is renamed, reformatted or reordered for its own sake.** A line of upstream's changes only when the change needs it.

**Upstream's root files are not edited**: `README.md`, `CONTRIBUTING.md`, `LICENSE.md`, `.gitignore` and `.github/`. What this fork has to say goes in [SOLENGINE.md](../../SOLENGINE.md), [CLAUDE.md](../../CLAUDE.md) and `docs/`, none of which is under `sdk/` or vendored.

**Every commit that changes `sdk/` is one change**, listed in SOLENGINE.md in the same commit, with the record that argues it.

## Why

**Every line we touch is a line a rebase can conflict on.** The fork stays cheap only while our commits are few and small against upstream's tree. A rename or a reformat touches lines no feature needed, and pays for it at every rebase after. Kept up, it turns the fork into a separate library nobody else maintains.

**Upstream's style is the one its maintainer reads.** A fix worth offering upstream is rewritten by a person anyway ([SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)). Written in upstream's shape, it is a small rewrite rather than a translation.

**`sdk/` is what SolEngine vendors.** Docs and notes outside it never reach the engine's tree, and never conflict with upstream, which keeps nothing at the root but its own few files and `sdk/`.

## Consequences

- **SolEngine's docs and record rules hold here, and its code rules do not.** [CLAUDE.md](../../CLAUDE.md) routes to each.
- **One commit per change** makes SOLENGINE.md's table the whole account of the fork, and lets a rebase stop on exactly the change that no longer applies.

## See also
- [0001-the-fork-follows-upstream](0001-the-fork-follows-upstream.md) — the rebase this keeps cheap
- [0002-an-engine-feature-is-off-by-default](0002-an-engine-feature-is-off-by-default.md) — the guard a feature is written behind
