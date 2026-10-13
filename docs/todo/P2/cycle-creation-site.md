# Where a collected object was created

**Ask** — in a build with SolEngine's tools, the library records where each garbage-collected script object was created, so the circular-reference callback can name a script position as well as a type. SolEngine's warning reads `The collector broke a reference cycle in script type 'Enemy'`, frames after the cycle was built, with nothing to say where ([SolEngine 0075](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0075-a-detected-cycle-warns-once-per-type.md)). With it, the warning names `AI.as:42`.

**Decided, not started** — Jeremi chose to do this on 2026-10-13. It is a feature behind `AS_SOL_` ([0002](../../decisions/0002-an-engine-feature-is-off-by-default.md)), which SolEngine turns on only where it compiles source.

**Open**
- What "created at" is: the line executing in the active context when the object is allocated. An object created from C++ has none.
- Where the position is stored. Per object costs memory on every collectable object; a side table keyed by address costs a lookup and a removal on free.
- How the callback receives it: a new query taking the object the callback is already handed, or a new callback signature. The query leaves upstream's callback unchanged.
- Whether it records only the section and line, or the whole call stack, which costs more and points at the cause rather than at a helper.

**Read**
- [SolEngine 0075](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0075-a-detected-cycle-warns-once-per-type.md) — the warning this feeds, and why it can name only a type today
- `sdk/angelscript/source/as_gc.cpp` — where a cycle is found and the callback fires
- `sdk/angelscript/source/as_scriptobject.cpp` — where a script object is allocated
- `sdk/angelscript/include/angelscript.h` — `asCIRCULARREFFUNC_t`
