# A collector step that says it has finished

**Ask** — an incremental `GarbageCollect(asGC_ONE_STEP)` returns whether there is work left, so a caller with a time budget can stop when the collector is done. Today it returns 1 whatever happened, and SolEngine paces its pass by the size of the collector's recent list instead ([SolEngine 0106](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0106-a-collector-pass-is-one-sweep.md)).

**An API addition upstream may take.** A new return value or a new query, rather than a change to an existing one, breaks no caller. Discuss it with upstream before writing it, as its `CONTRIBUTING.md` asks of anything larger than a fix.

**Open**
- A new return code from `GarbageCollect`, or a separate "is there pending work" query.

**Read**
- [SolEngine 0106](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0106-a-collector-pass-is-one-sweep.md) — the work-paced pass this would replace
- `sdk/angelscript/source/as_gc.cpp` — the step and its return
