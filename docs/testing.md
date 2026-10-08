# Testing

Scope: what a release of this fork passes, and what each run proves. When a release is cut is [0001](decisions/0001-the-fork-follows-upstream.md)'s. That SolEngine vendors only a release that passed is [SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)'s.

## The release check, in order

1. **Upstream's suite, every feature off.** `sdk/tests/test_feature`, built the way SolEngine builds the library, by `check/Check.ps1`. Proves the library still does what upstream's does, under our compilers and flags ([0002](decisions/0002-an-engine-feature-is-off-by-default.md)).
2. **Each feature's own tests, with it on.** Proves the feature does what its record says. There are no features yet, so this step is empty.
3. **SolEngine's full gate**, all six configurations, against the vendored tree. Proves the engine still builds and passes with this release. It is run in SolEngine, by [its testing doc](https://github.com/Jeremi247/SolEngine/blob/main/docs/testing.md).

A release is tagged only when all three pass.

## Step 1

```bash
powershell -ExecutionPolicy Bypass -File check/Check.ps1
```

Four runs: MSVC and clang-cl, each Debug and Development, through the Visual Studio 2022 generator as SolEngine's presets use it. `-Toolset` and `-Configuration` narrow it. Each run's output is `check/Build/<toolset>-<configuration>.log`, and the script exits non-zero if any run failed.

**A run passes when the suite exits 0 and the library reports `AS_NO_EXCEPTIONS`.** The second half is 0076's rule that a definition which stops firing has to be visible: without it, the tests that skip themselves under that definition would come back on, and the run would still pass.

**The library half is SolEngine's, the suite half is upstream's.** `check/CMakeLists.txt` compiles the library from the source list, definitions and `SolThirdPartyFlags` in SolEngine's `ThirdParty/AngelScript/CMakeLists.txt`, at C++20 and with SolEngine's Debug and Development flags. That list is kept in step with SolEngine's by hand. The suite's sources are read out of upstream's own `test_feature` CMakeLists, so a test upstream adds needs no edit here. The suite and the add-ons are compiled with RTTI and exceptions on, as upstream compiles them: they are not shipped, and clang-cl refuses their `try` and `dynamic_cast` without.

**Development stands for Shipping too.** The two differ only in `/Zi`.

At `c9e74722`, all four runs pass 158 tests. Three are skipped by the suite itself: two because the library reports `AS_NO_EXCEPTIONS`, one for a missing glm. Under clang-cl a fourth, `WRAP_MFN_PR`, is skipped as it is for clang upstream.

## What upstream already runs, and what it does not

Upstream's `.github/workflows/auto-test.yml` builds and runs `test_feature` on every push to its `master`. On Windows that is clang and clang-cl, Debug and Release, through upstream's own CMakeLists. It also runs on Linux and macOS, which SolEngine does not target.

None of it is SolEngine's build:

| | SolEngine | Upstream's CI |
|---|---|---|
| Compiler | MSVC `cl.exe` and clang-cl | clang and clang-cl |
| `AS_NO_EXCEPTIONS` | Always | Under clang only, set by upstream's CMakeLists |
| RTTI and exceptions | Off (`/GR- /EHs-c-`) | Compiler default |
| Allocation | Through `asSetGlobalMemoryFunctions`, a size header on every block | Through `asSetGlobalMemoryFunctions`, the suite's own `malloc` wrapper |

Step 1 exists for those differences. A green upstream CI says nothing about them.

**The allocator is the suite's, not SolEngine's.** The suite installs its own memory functions for the whole run, and `test_compiler.cpp` resets them to the default part way through, so SolEngine's header-carrying allocator cannot be put in their place without editing upstream's tests. The header is covered by step 3, where SolEngine's `Tests/Script/` asserts a released engine returns every byte.

**`AS_NO_COMPILER` is out of reach of step 1.** SolEngine's builds without tools configure the compiler out, and the suite compiles scripts. No gate build covers it either: SolEngine's six all keep tools on ([its testing doc](https://github.com/Jeremi247/SolEngine/blob/main/docs/testing.md)). A release builds a tools-off tree by hand, `-DSOL_WITH_TOOLS=OFF` ([build](https://github.com/Jeremi247/SolEngine/blob/main/docs/build.md)), and runs its tests.

## See also
- [0002-an-engine-feature-is-off-by-default](decisions/0002-an-engine-feature-is-off-by-default.md) — why the suite runs with features off
- [SolEngine's vendoring record](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md) — the flags and definitions step 1 reproduces
- [P0](todo/P0.md) — the first release, which this check gates
