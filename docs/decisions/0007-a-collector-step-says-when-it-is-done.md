# 0007 — A collector step says when it is done

Scope: `AS_SOL_GC_REPORTS_DONE`, the feature that lets a caller with a time budget stop stepping the collector when it has nothing left to do ([SolEngine 0106](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0106-a-collector-pass-is-one-sweep.md)). How SolEngine paces its pass with it is SolEngine's.

## The decision

**An incremental `GarbageCollect` returns 0 once the collector has nothing left to do, and 1 otherwise.** No new function. A full cycle is unchanged, and already returned 0.

- **"Nothing left to do" means each incremental step the call runs has ended a pass that found nothing.** The steps are the destruction of new garbage, the destruction of old garbage, and the detection of cycles.
- **A pass counts only if nothing changed while it ran, and nothing has changed since it ended.** A change is an object destroyed, detected in a cycle, added or aged into the old list.
- **A call with several iterations stops at the first one that is done.**
- **The call after a 0 starts over**, so a caller stepping every frame sweeps every frame.

## Why

**Upstream's three steps never line up.** Each is a state machine that examines one object per call and returns 0 only on the call that ends a quiet pass. The three end their passes at different calls, so no single call can say all three are quiet. Each step's last quiet pass is remembered, and a signature of the collector's counters says whether it still holds.

**The counters are what can make a pass stale.** A destruction can bring another object to a count of one. An object aged into the old list has not been through a detection. An object added has not been examined. All of these move a counter the collector already keeps, or the length of the old list.

**A release by the application changes no counter, so done cannot last.** An object examined live may become garbage the moment the caller drops it, and the collector is not told. Starting over after each 0 keeps that garbage from being missed for good. It costs one quiet pass of each list per round, which is the sweep SolEngine paces by today.

**A return value, not a query.** Upstream's documentation already reads the return as "1 if the cycle was not finished". This makes the incremental case honour it, and breaks no caller that ignores it.

## Consequences

- **SolEngine can step until 0 or its budget, whichever comes first**, instead of counting steps by the size of the recent list.
- **Freeing a cycle can take more than one round.** A round that ages objects into the old list is not quiet, so the detection that finds them ends a later round.
- **Six tests hold it** in `check/Features/GcReportsDone.cpp`: an empty collector, garbage, a live object held by the application, cycles, and iterations stopping early.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [SolEngine 0106](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0106-a-collector-pass-is-one-sweep.md) — the pass this lets end early
