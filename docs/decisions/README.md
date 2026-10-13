# Decisions

Scope: one line per decision — what was decided, and which record to trust when one supersedes another. The argument is in the record; open it. Kept by SolEngine's rules ([docs & comments](https://github.com/Jeremi247/SolEngine/blob/main/docs/conventions/docs-and-comments.md)): append-only in substance, not in prose.

Numbered on their own. A SolEngine record is named *SolEngine 0110* and linked by URL.

- [0001 — The fork follows upstream](0001-the-fork-follows-upstream.md) — `master` mirrors upstream and is never committed to; `sol/main` is `master` plus our commits, rebased rather than merged; a release is a tag `sol-<upstream version>-<n>` on `sol/main`, never moved, and is the only stable reference
- [0002 — An engine feature is off by default](0002-an-engine-feature-is-off-by-default.md) — a feature is behind `AS_SOL_<FEATURE>`, reported by `asGetLibraryOptions()`, turned on by SolEngine's build, and tested by its own suite; upstream's suite runs with every feature off; a fix to upstream's behaviour is not gated
- [0003 — A commit of ours keeps upstream's shape](0003-a-commit-of-ours-keeps-upstreams-shape.md) — `sdk/` is written in upstream's style; a new file over an edit; nothing renamed or reformatted for its own sake; upstream's root files untouched; one change per commit, listed in `SOLENGINE.md`
- [0004 — A feature is tested beside upstream's suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — a feature's tests are a file in `check/Features/`, run against a second build with every feature on, compiled as SolEngine compiles its code; its interface is `angelscript_sol.h` and its code `as_sol.cpp`; a feature is a `PUBLIC` definition
- [0005 — A declaration reports where it was made](0005-a-declaration-reports-where-it-was-made.md) — `AS_SOL_DECLARATION_POSITIONS`: script types, properties and globals report the section, row and column of their name; source only, `asNOT_SUPPORTED` otherwise
- [0006 — A collected object says where it was created](0006-a-collected-object-says-where-it-was-created.md) — `AS_SOL_CYCLE_CREATION_SITE`: the collector's entry for an object keeps the line executing in the innermost script function with lines; a query, not a new callback; one position, not a stack
- [0007 — A collector step says when it is done](0007-a-collector-step-says-when-it-is-done.md) — `AS_SOL_GC_REPORTS_DONE`: an incremental `GarbageCollect` returns 0 once each step it runs has ended a pass that found nothing, with nothing changed since; it stops iterating there, and the next call starts over
- [0008 — `const` reaches through a handle member](0008-const-reaches-through-a-handle-member.md) — `AS_SOL_DEEP_CONST`, a dialect: a handle member read through a `const` object or `this` in a `const` method is a handle to `const`; `F@ const`, funcdefs and returned handles are unchanged
- [0009 — A module can be told to hold no state](0009-a-module-can-be-told-to-hold-no-state.md) — `AS_SOL_NO_SCRIPT_STATE`: a class implementing an interface the application names declares no members, and with the rule on a global is a constant of a primitive, enum or POD value type; compile errors at the declaration, source only, off until asked
- [0010 — A value type crosses as `&inout` safely](0010-a-value-type-crosses-as-inout-safely.md) — `AS_SOL_SAFE_VALUE_INOUT`: with unsafe references off a value type may be `&inout`; locals, parameters and safe references go by address, a property chain as a copy assigned back after the call, anything else is refused

## See also
- [CLAUDE.md](../../CLAUDE.md) — where each record is read
- [SolEngine's register](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/README.md) — why the fork exists, and what the engine needs from it
