# A free callback that is told the size

**Ask** — a variant of `asSetGlobalMemoryFunctions` whose free takes the block's size, as the allocate already does. SolEngine's allocator interface needs the size to deallocate, so today every block the library allocates carries a 16-byte header holding it ([SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md)).

**An API addition upstream may take**, but only if every free site in the library knows the size it allocated, which is the whole of the work. Discuss with upstream first.

**Open**
- How many of the library's frees know their size today. A free site that does not means tracking the size, which is the header moved inside the library.
- Whether 16 bytes a block is worth it. Script allocates rarely by design ([SolEngine 0019](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0019-scripting-language.md)).

**Read**
- [SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md) — the header, and why the payload needs 16 bytes rather than 8
- `sdk/angelscript/source/as_memory.cpp` — the global functions
