# Cut `sol-2.39.0-wip.1` and vendor it into SolEngine

**Ask** — tag upstream `825931ef`, carried on `sol/main` with no commits of ours under `sdk/`, as `sol-2.39.0-wip.1`, once the release check passes. Then replace SolEngine's `ThirdParty/AngelScript/` with it. That brings in 239 upstream commits past 2.38.0, including the `const` fix SolEngine 0109 found.

**In SolEngine, with the vendoring**
- `ThirdParty/AngelScript/README.md`: the provenance row names this tag and `825931ef`, and the upgrade steps name this fork. Upstream now ships `LICENSE.md`, which the README says it does not.
- `ExpectedVersion` in `Tests/Script/ScriptLibraryTests.cpp` moves to the number the new header declares.
- `ThirdParty/AngelScript/CMakeLists.txt`: one source file is new since 2.38.0, `as_callfunc_x64_msvc_clang.S`, an assembler file for clang on Windows. SolEngine assembles `as_callfunc_x64_msvc_asm.asm` with `ml64` under clang-cl today ([SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md)), so whether the new file replaces that under clang-cl is a decision, not a list edit.
- 0109's consequence on the unqualified-call bug says the fix is vendored.
- The full gate, all six configurations.

**Blocked on** — [building upstream's suite](build-test-feature.md).

**Read**
- [0001](../../decisions/0001-the-fork-follows-upstream.md) — the tag's name, and why it is never moved
- [SOLENGINE.md](../../../SOLENGINE.md) — the release table this fills in
- [SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md) — what a vendored release is
