# 0004 — A feature is tested beside upstream's suite

Scope: where the tests of an `AS_SOL_` feature live, how they are built and how the release check runs them. [0002](0002-an-engine-feature-is-off-by-default.md) deferred this to the first feature. What each feature does is its own record's.

## The decision

**A feature's tests are a file in `check/Features/`, named for the feature**, compiled into one executable, `test_sol_features`.

**They run against a second build of the library with every `AS_SOL_` feature on.** `check/CMakeLists.txt` builds the library twice from the same source list: once as SolEngine builds it today, for upstream's suite, and once with the definitions in `SolFeatures` set `PUBLIC`, for the feature tests.

**The tests are compiled as SolEngine compiles its own code**: C++20, `/W4 /WX`, no RTTI and no exceptions.

**`check/Check.ps1` runs both in every configuration** and reports them as separate rows. A run of the feature tests passes when it exits 0. Each feature's file asserts that `asGetLibraryOptions()` reports it.

**A feature's interface is declared in `angelscript_sol.h`, and its code lives in `as_sol.cpp`** wherever it can, beside the guarded calls 0003 allows into upstream's files.

## Why

**All features on is the configuration SolEngine runs.** A feature tested alone could pass while it breaks beside another, and SolEngine never builds one alone. Upstream's suite still covers the build with every feature off, which is the claim 0002 needs.

**Outside `sdk/`, the tests are never vendored and never conflict with upstream.** Upstream's `test_feature` was the alternative home. Adding a file there edits upstream's CMakeLists and its test list, which a rebase then has to carry.

**`PUBLIC` because the header asks for it.** `angelscript_sol.h` declares a feature's functions only where the definition is set. A consumer without the definition gets a compile error rather than a call into a library that lacks the feature. That is SolEngine's position too, so the tests are built the same way.

**A new header rather than `angelscript.h`.** Methods added to upstream's interfaces would change their vtables under a definition, and a consumer built without it would call the wrong slot. Free functions in a header of our own fail at compile or link time instead, and leave upstream's header unedited.

## Consequences

- **SolEngine's source list gains `as_sol.cpp`**, and its include gains `angelscript_sol.h`. With no feature set, the file compiles to nothing.
- **A feature SolEngine turns on is a `PUBLIC` definition in its build**, not a private one.
- **[Testing](../testing.md)'s step 2 is this executable.**

## See also
- [0002-an-engine-feature-is-off-by-default](0002-an-engine-feature-is-off-by-default.md) — the two configurations this tests
- [0003-a-commit-of-ours-keeps-upstreams-shape](0003-a-commit-of-ours-keeps-upstreams-shape.md) — why a new file over an edit
- [Testing](../testing.md) — the release check
