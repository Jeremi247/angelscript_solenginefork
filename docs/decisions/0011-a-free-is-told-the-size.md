# 0011 — A free is told the size

Scope: `AS_SOL_SIZED_FREE`, a variant of `asSetGlobalMemoryFunctions` whose free takes the block's size. SolEngine's allocator needs the size to deallocate, and today keeps it in a 16-byte header on every block ([SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md)). Dropping that header is SolEngine's.

## The decision

**The application can give the library a free function that takes the size the block was allocated with.** Two functions in `angelscript_sol.h`:

```cpp
typedef void (*asSIZEDFREEFUNC_t)(void *, size_t);
int  asSolSetGlobalMemoryFunctions(asALLOCFUNC_t allocFunc, asSIZEDFREEFUNC_t freeFunc);
void asSolFreeMem(void *mem, size_t size);
```

- **Every free in the library passes the size it allocated.** With the feature on, the unsized `asDELETEARRAY` and `CallFree(void*)` do not exist, so a free that does not know its size fails to compile.
- **Two kinds of block keep a header of their own.** A list initialisation buffer, which is short-lived, and a block from `asAllocMem`, which the application frees by pointer alone. Both are 16 bytes, which keeps the payload aligned.
- **`asSetGlobalMemoryFunctions` and `asResetGlobalMemoryFunctions` return to an unsized free**, which every free then falls back to.
- **x64 MSVC only.** The other platforms' calling convention files call the unsized `CallFree`, and do not compile with the feature on.

## Why

**Almost every free already knew its size, so the work was making it say so.** An object is deleted as its type, an array keeps its capacity, a context's stack blocks double in a known order, and a script object's size is in its type. Upstream discards all of it at the free. Where a site did not know, the alternative was the header moved inside the library, so each one was fixed instead:

- **`asCString` lost its capacity when a long string shrank.** With the feature, it reallocates on shrink, so its buffer is always its length plus one.
- **A type, and a list pattern node, are freed through a base class.** Each is now freed with the size of the class it was created as, told apart by the flags or kind its creator gave it.
- **A script object's size is read before its destructor runs**, which can release the type.

**A list buffer keeps a header because nothing else knows its size.** The compiler sizes it from the list, and frees it as a list pattern type, which has no size. It lives for one statement.

**The removed forms are the test that nothing was missed.** A free site added upstream fails to compile here rather than freeing with the wrong size.

## Consequences

- **SolEngine can drop its 16-byte header** and pass the size to `IAllocator::Deallocate`, once it calls `asSolSetGlobalMemoryFunctions` before creating an engine.
- **A rebase that adds a free upstream stops at it.** That is the feature working, and the fix is to pass the size.
- **Four tests hold it** in `check/Features/SizedFree.cpp`. A workload is run against an allocator that checks every free's size: compiling, cycles, value objects, list buffers, a deep stack, exceptions while unwinding, globals, and bytecode saved and loaded. Every block comes back at the size it was allocated. The other tests cover `asAllocMem` and the fallback to an unsized free.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md) — the header this replaces
