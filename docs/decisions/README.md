# Decisions

Scope: one line per decision — what was decided, and which record to trust when one supersedes another. The argument is in the record; open it. Kept by SolEngine's rules ([docs & comments](https://github.com/Jeremi247/SolEngine/blob/main/docs/conventions/docs-and-comments.md)): append-only in substance, not in prose.

Numbered on their own. A SolEngine record is named *SolEngine 0110* and linked by URL.

- [0001 — The fork follows upstream](0001-the-fork-follows-upstream.md) — `master` mirrors upstream and is never committed to; `sol/main` is `master` plus our commits, rebased rather than merged; a release is a tag `sol-<upstream version>-<n>` on `sol/main`, never moved, and is the only stable reference
- [0002 — An engine feature is off by default](0002-an-engine-feature-is-off-by-default.md) — a feature is behind `AS_SOL_<FEATURE>`, reported by `asGetLibraryOptions()`, turned on by SolEngine's build, and tested by its own suite; upstream's suite runs with every feature off; a fix to upstream's behaviour is not gated
- [0003 — A commit of ours keeps upstream's shape](0003-a-commit-of-ours-keeps-upstreams-shape.md) — `sdk/` is written in upstream's style; a new file over an edit; nothing renamed or reformatted for its own sake; upstream's root files untouched; one change per commit, listed in `SOLENGINE.md`

## See also
- [CLAUDE.md](../../CLAUDE.md) — where each record is read
- [SolEngine's register](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/README.md) — why the fork exists, and what the engine needs from it
