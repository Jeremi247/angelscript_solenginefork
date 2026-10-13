# 0008 — `const` reaches through a handle member

Scope: `AS_SOL_DEEP_CONST`, a change to the language rather than the interface: what `const` means for the object a handle member refers to. It closes two of the three holes [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) found in `const`. The third, an unqualified call to a non-`const` method, is fixed upstream. Whether SolEngine relaxes 0109 because of this is SolEngine's.

## The decision

**A handle member read through a `const` object is a handle to a `const` object.** That covers `this` in a `const` method, a `const` script object, and a handle to `const`. In a `const` method, with `F@ H` a member:

- `H.X = 1`, `this.H.X = 1` and `H.Bump()` are refused.
- `Value.H.X = 1` is refused when `Value` is a value member holding a handle.
- `F@ f = H;` is refused, and `const F@ f = H;` compiles.
- Reading `H.X`, and calling `const` methods on `H`, compile as before.

**Only the handle member changes.** The member itself was already read-only, and stays so.

**Not changed:**

- **`F@ const`**, a handle that cannot be reseated. It says nothing about the object, and still allows writing through it.
- **A funcdef member.**
- **A handle returned by a method or a property accessor.** Its declared type stands.

## Why

**A dialect, accepted for the engine that asked for it.** Upstream's `const` is shallow, as C++'s is. Under the feature, a script relying on writing through a handle member of a `const` object stops compiling. That breaks code, but never changes what compiled code does. SolEngine's script is new, and has none of it.

**Deep `const` stops at the member, not at every handle.** `F@ const` is the author saying the object is mutable, and an accessor's return is the author choosing a type. Reaching through either would override a declaration rather than follow one.

**Recursion is free.** Each step of `a.b.c` asks whether the object on its left is `const`. Making one handle member's object `const` makes the next step's object `const` too, with no walk.

**Funcdefs are left out** because a funcdef handle has no `const` form to call through.

## Consequences

- **SolEngine could allow members on a system as read-only configuration, with `Tick` declared `const`.** 0109 refused that partly because of these holes. Its other reasons stand, and whether to revisit it is SolEngine's call.
- **A container member is as deep as its registered `const` overloads.** An array's `const` `opIndex` returns `const` elements.
- **Four tests hold it** in `check/Features/DeepConst.cpp`: the refusals through `this`, through `const` objects, each checked for the right error, and the reads and mutable access that still compile and run.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [0002-an-engine-feature-is-off-by-default](0002-an-engine-feature-is-off-by-default.md) — why a language change is a feature
- [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) — the holes, each probed against 2.38.0
