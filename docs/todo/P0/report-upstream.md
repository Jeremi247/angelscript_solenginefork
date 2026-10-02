# Report two defects upstream

**Ask** — two defect reports on [anjo76/angelscript](https://github.com/anjo76/angelscript/issues), filed and worded by a person ([SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)). Both are present on upstream's `master` at `825931ef`, and neither had an issue as of 2026-10-02.

1. **`asGetTypeTraits` and MSVC x64 aggregates.** A struct with default member initialisers and no declared constructor gets `asOBJ_APP_CLASS_CONSTRUCTOR`. MSVC x64 returns such a type of 1, 2, 4 or 8 bytes in `RAX`, but `as_config.h` counts the flag as a return in memory, so a native call reads a result nothing wrote. Found in Shipping only, where SolEngine registers native calls rather than generic wrappers ([SolEngine 0103](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0103-an-aggregate-is-returned-in-a-register.md)).
2. **The universal `opCast(?&out)` path does not check its context.** In `asCScriptEngine::RefCastObject`, the block marked `// TODO: Add proper error handling` calls `ctx->Prepare` on the result of `RequestContext()` without a null check. The library's seven other requests check it ([SolEngine 0084](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0084-the-dispatch-context-pool.md)). Low severity: it needs a context callback that returns null.

**Not a fork commit.** The first is a fix to upstream's behaviour, and would apply ungated ([0002](../../decisions/0002-an-engine-feature-is-off-by-default.md)) if it is ever carried here. SolEngine's workaround makes that unnecessary today.

**Read**
- `sdk/angelscript/include/angelscript.h` — `asGetTypeTraits`
- `sdk/angelscript/source/as_config.h` — `COMPLEX_RETURN_MASK` under MSVC x64
- `sdk/angelscript/source/as_scriptengine.cpp` — the `universalCastFunc` block
