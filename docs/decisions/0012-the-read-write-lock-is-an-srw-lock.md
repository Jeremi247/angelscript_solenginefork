# 0012 — The read-write lock is an SRW lock

Scope: `AS_SOL_SRW_LOCK`, which makes `asCThreadReadWriteLock` an `SRWLOCK` on Windows. It covers both instances, each engine's `engineRWLock` and the thread manager's `appRWLock` behind `asAcquireSharedLock`. SolEngine's collector takes the engine's lock every frame ([SolEngine 0134](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0134-a-collector-pass-runs-until-the-collector-is-done.md)). POSIX already uses `pthread_rwlock_t` and is unchanged.

## The decision

**On Windows the lock is an `SRWLOCK`.** No new function. A take that finds the lock free makes no kernel call, and any number of threads may hold it shared.

- **It is not recursive.** A thread holding the lock must not take it again, shared or exclusive. The library never does ([below](#why-no-recursive-take-exists)).
- **With asserts on, the library asserts it.** Each thread keeps a short list of the locks it holds, and a take of one already in it asserts before it blocks. `NDEBUG` compiles the list out.
- **Windows Vista or later**, which `as_criticalsection.h` already targets with `_WIN32_WINNT 0x0600`.

## Why

**Upstream's lock is a semaphore of ten reader slots and a critical section.** A shared take is `WaitForSingleObjectEx` and its release `ReleaseSemaphore`, two kernel calls whether or not anyone else holds the lock. An eleventh concurrent reader waits. Upstream's own comment leaves a `TODO` to allow `SRWLOCK` by configuration; it kept the semaphore for Windows XP.

**Upstream's lock was never safe to take twice either.** A writer drains the slots one at a time. A reader holding one slot and asking for a second waits on a writer that waits on it, the same deadlock an SRW lock has. A second exclusive take deadlocks on its own slots. So the rule the SRW lock needs is one the library already had to keep.

## Why no recursive take exists

Every `ACQUIRESHARED`, `ACQUIREEXCLUSIVE` and `asAcquire*Lock` in `sdk/angelscript/source/` was read at `c9e74722`. Each region between a take and its release does array or map work, `asNEW` and string compares, and calls nothing that takes a lock:

- **The user data getters and setters** of the engine, module, context, function, type and script object, and the five cleanup callback setters.
- **`DeleteDiscardedModules`** reads the discarded list under shared and releases before it deletes a module, whose destructor takes exclusive.
- **`GetModule`** releases shared before it takes exclusive to set `lastModule`. No path takes exclusive while holding shared.
- **The cleanup callbacks** run with no lock held, as does everything in `asCGarbageCollector`, which has critical sections of its own.

The application's memory functions are called inside a region, through `asNEW` and an array's growth, and must not call back into the library.

## Consequences

- **SolEngine's per-frame `GarbageCollect` and `DeleteDiscardedModules` make no kernel call** for the lock while no other thread holds it exclusive.
- **A rebase that adds a take inside a region** is caught by the assert in any Debug run that reaches it.
- **Four tests hold it** in `check/Features/SrwLock.cpp`: the option is reported, sixteen readers hold the lock at once, a writer excludes a reader, and an engine is built, run, discarded and collected while four threads read. With the definition removed, the first two fail.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [SolEngine 0134](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0134-a-collector-pass-runs-until-the-collector-is-done.md) — the pass that takes the lock every frame
