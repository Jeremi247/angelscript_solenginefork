# Testing

Scope: what a release of this fork passes, and what each run proves. When a release is cut is [0001](decisions/0001-the-fork-follows-upstream.md)'s. That SolEngine vendors only a release that passed is [SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)'s.

**Not runnable yet.** Building upstream's suite under SolEngine's toolchain is [P0](todo/P0.md). This doc says what the runs are for, so the build is made to answer that.

## The release check, in order

1. **Upstream's suite, every feature off.** `sdk/tests/test_feature`, built the way SolEngine builds the library. Proves the library still does what upstream's does, under our compilers and flags ([0002](decisions/0002-an-engine-feature-is-off-by-default.md)).
2. **Each feature's own tests, with it on.** Proves the feature does what its record says. There are no features yet, so this step is empty.
3. **SolEngine's full gate**, all six configurations, against the vendored tree. Proves the engine still builds and passes with this release. It is run in SolEngine, by [its testing doc](https://github.com/Jeremi247/SolEngine/blob/main/docs/testing.md).

A release is tagged only when all three pass.

## What upstream already runs, and what it does not

Upstream's `.github/workflows/auto-test.yml` builds and runs `test_feature` on every push to its `master`. On Windows that is clang and clang-cl, Debug and Release, with upstream's default configuration. It also runs on Linux and macOS, which SolEngine does not target.

None of it is SolEngine's build, which differs in four ways:

| | SolEngine | Upstream's CI |
|---|---|---|
| Compiler | MSVC `cl.exe` and clang-cl | clang and clang-cl |
| `AS_NO_EXCEPTIONS` | Always | Never |
| RTTI and exceptions | Off (`/GR- /EHs-c-`) | Compiler default |
| Allocation | Through `asSetGlobalMemoryFunctions` | Library default |

Step 1 exists for those four differences. A green upstream CI says nothing about them.

**`AS_NO_COMPILER` is out of reach of step 1.** SolEngine's builds without tools configure the compiler out, and the suite compiles scripts. That configuration is covered by step 3 alone.

## See also
- [0002-an-engine-feature-is-off-by-default](decisions/0002-an-engine-feature-is-off-by-default.md) — why the suite runs with features off
- [SolEngine's vendoring record](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md) — the flags and definitions step 1 has to reproduce
- [P0](todo/P0.md) — building it
