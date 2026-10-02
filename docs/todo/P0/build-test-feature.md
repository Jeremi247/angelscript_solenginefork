# Build and run upstream's suite the way SolEngine builds the library

**Ask** — `sdk/tests/test_feature` built and run on Windows x64 under MSVC and clang-cl, Debug and Release, with the library compiled as SolEngine compiles it: `AS_NO_EXCEPTIONS`, `/GR- /EHs-c-`, and allocation routed through `asSetGlobalMemoryFunctions`. One command, documented in [CLAUDE.md](../../../CLAUDE.md) and [testing](../../testing.md). Not in it: Linux, macOS, or any feature.

**Open**
- Drive upstream's `projects/cmake` with flags passed in, or write a CMakeLists of ours outside `sdk/`. Ours can mirror SolEngine's `ThirdParty/AngelScript/CMakeLists.txt`, which is what is actually shipped, but it is a second list of sources to keep in step.
- Whether the suite passes at all under `AS_NO_EXCEPTIONS`. Upstream's CI never builds it that way, so a test that throws or catches may fail for a reason that is the suite's and not the library's. Such a test is excluded with a line saying why, not fixed in place ([0003](../../decisions/0003-a-commit-of-ours-keeps-upstreams-shape.md)).
- Where a feature's own tests will live, and whether they share this build ([0002](../../decisions/0002-an-engine-feature-is-off-by-default.md)).

**Blocked on** — nothing.

**Read**
- [testing](../../testing.md) — the four ways SolEngine's build differs from upstream's CI
- [SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md) — the flags, definitions and allocator to reproduce
- SolEngine's `ThirdParty/AngelScript/CMakeLists.txt` — the source list and why four files are not compiled
- `sdk/tests/test_feature/projects/cmake/CMakeLists.txt` and `.github/workflows/auto-test.yml` — how upstream builds it
