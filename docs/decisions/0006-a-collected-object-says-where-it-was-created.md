# 0006 — A collected object says where it was created

Scope: `AS_SOL_CYCLE_CREATION_SITE`, the feature that lets SolEngine's cycle warning name a script position as well as a type ([SolEngine 0075](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0075-a-detected-cycle-warns-once-per-type.md)). Jeremi chose to build it on 2026-10-13. What the warning prints is SolEngine's.

## The decision

**When the collector is told of a new object, it records the line executing in the active context**, in the innermost script function that has line numbers. One function in `angelscript_sol.h` reads it back:

```cpp
int asSolGetObjectCreatedAt(asIScriptEngine *engine, void *obj, const char **scriptSection, int *row, int *col);
```

- **The position is stored in the collector's own entry for the object**, beside its type and sequence number.
- **It is a query, not a new callback.** The circular reference callback is unchanged, and is already handed the object to ask about.
- **One position, not a call stack.**
- **An object created with no script executing returns `asNOT_SUPPORTED`.** An object the collector does not hold returns `asINVALID_ARG`.
- **SolEngine turns it on only where it compiles source**, beside its tools.

## Why

**The collector's entry costs nothing new to find or free.** A side table keyed by address would need a lookup on every creation and a removal on every free. A field on every script object would cost memory on objects that are never collected. The entry exists only for collectable objects, and goes when they do. It grows from 24 to 32 bytes on x64.

**The innermost frame with lines is the line that called the constructor.** A script class is created through a factory stub that has no line numbers of its own, so the frame beneath it is the expression that asked for the object. A helper like `Make()` is named rather than its caller. That points at the code to read, and a call stack would cost a copy per object to point further.

**A query, because the lookup is rare.** It walks the collector's lists, which is linear. SolEngine asks once per type, on the first detection ([SolEngine 0075](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0075-a-detected-cycle-warns-once-per-type.md)).

## Consequences

- **SolEngine's warning can name `AI.as:42`**, the line that created one object of the cycle. It is not necessarily the line that closed the cycle.
- **Every collectable object costs eight bytes more, and its creation one walk of the context's stack**, in a build with the feature on.
- **Three tests hold it** in `check/Features/CycleCreationSite.cpp`: a cycle names the line in the helper that created one node and the declaration that created the other, an object created from C++ has no position, and an unknown object is refused.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [SolEngine 0075](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0075-a-detected-cycle-warns-once-per-type.md) — the warning this feeds
