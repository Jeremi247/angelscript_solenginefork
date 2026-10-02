# 0001 — The fork follows upstream

Scope: the branches, how upstream's changes come in, and what a release tag is. Why there is a fork at all, and what a release must pass before SolEngine vendors it, are [SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)'s. This records how this repository carries them out.

## The decision

**`master` mirrors upstream.** It is fast-forwarded from `anjo76/angelscript`'s `master` and never committed to, so it always answers "what is upstream" without a diff.

**`sol/main` is `master` plus our commits**, and is the branch work happens on. Upstream's changes come in by rebasing `sol/main` onto a fast-forwarded `master`. Merging upstream into `sol/main` is not done.

**A release is a tag on `sol/main`**, named `sol-<upstream version>-<n>`. `<upstream version>` is the one upstream's header declares, so a tree built from upstream's work in progress reads `sol-2.39.0-wip.<n>`. `<n>` counts our releases on that version from 1.

**A tag is never moved or deleted.** SolEngine's vendored tree names one, and must be able to find it again.

## Why

**A rebase keeps our commits a list.** After a merge, what we changed is spread across merge resolutions and has to be read out of a diff against upstream. After a rebase, it is the commits between `master` and `sol/main`, each one ours, each one applying to upstream as it stands today. That list is what [SOLENGINE.md](../../SOLENGINE.md) states and what the licence's "plainly marked" asks for. A commit that no longer applies stops the rebase at the point that needs a person.

**The cost is a rewritten branch.** Every rebase rewrites `sol/main`, so it is force-pushed, and anything built on an old `sol/main` commit is stranded. The tags are why that is acceptable: a release keeps its commits reachable whatever `sol/main` becomes, and nothing else should be built on.

**A mirrored `master` is free.** It costs one fast-forward and makes "is this ours or upstream's?" a `git log master..sol/main`.

## Consequences

- **`sol/main` is force-pushed after every rebase.** Nothing but a tag is a stable reference.
- **A commit of ours that upstream absorbs disappears at the next rebase**, as an empty commit git drops. Its row in SOLENGINE.md goes with it.
- **The first release is `sol-2.39.0-wip.1`**, at upstream `825931ef` with no commits of ours under `sdk/` ([P0](../todo/P0.md)).

## See also
- [0002-an-engine-feature-is-off-by-default](0002-an-engine-feature-is-off-by-default.md) — what a commit of ours adds
- [0003-a-commit-of-ours-keeps-upstreams-shape](0003-a-commit-of-ours-keeps-upstreams-shape.md) — how it adds it
- [Testing](../testing.md) — what a release passes
