# `&inout` for a value type without unsafe references

**Ask** — a registered value type may cross as `&inout` with `asEP_ALLOW_UNSAFE_REFERENCES` off. Today the builder accepts `&inout` only on an object with `addref` and `release` or `asOBJ_NOCOUNT`, which a plain value type has neither of, so SolEngine turned the property on for the whole engine to pass a written component ([SolEngine 0083](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0083-a-written-component-crosses-as-inout.md)).

**What the property allows today** — a reference into an object that dies during the call. Untested; on reading, this compiles and writes to freed memory:

```angelscript
class Box { FPosition P; }
void Bump(FPosition &inout P, Box@ &inout B) { @B = null; P.X = 1; }
Box@ b = Box();
Bump(b.P, b);
```

`Grow(Points[0], Points)` is the same fault once an array is registered.

**The proposed rule** — a value-type `&inout` argument that is a local is passed by address. Any other argument goes through a temporary that is written back when the call returns, so `Bump` above writes to the copy. It accepts only code safe mode refuses, so no script valid upstream changes meaning. A feature behind `AS_SOL_` ([0002](../../decisions/0002-an-engine-feature-is-off-by-default.md)).

**Due before** SolEngine registers its first reference type, the array 0083 names as its trigger.

**Open**
- A SolEngine test showing the fault above with the property on, and gone under the feature.
- Whether a local is the whole safe set. A parameter that is itself an `&inout` reference into a dying object is the case to rule out.
- What the write-back costs on a large value type.

**Read**
- [SolEngine 0083](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0083-a-written-component-crosses-as-inout.md) — the property, and the trigger to re-read it
- [SolEngine 0085](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0085-the-dispatcher-refuses-at-registration.md) — the aliasing the property allows, bounded at the dispatch entry
- [SolEngine 0079](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0079-deducing-a-binding-declaration.md) — where the builder's requirement was found
- `sdk/angelscript/source/as_builder.cpp` — the check
- `sdk/angelscript/source/as_compiler.cpp` — where an argument becomes a temporary
