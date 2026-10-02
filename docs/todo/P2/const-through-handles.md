# `const` that reaches through a handle

**Ask** — inside a `const` method, or through a `const` object, a handle member yields a handle to `const`. Today `const` stops at the handle. With `F@ H` a member, `H.X = 1` compiles in a `const` method, as it does through a handle nested in a value member, and through `F@ const` or a `const` script object held in a global ([SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md)).

**A language change, not a fix.** Upstream's `const` is shallow by design, as C++'s is. Deep `const` makes every script relying on the shallow kind stop compiling, so it is a feature behind `AS_SOL_` ([0002](../../decisions/0002-an-engine-feature-is-off-by-default.md)) and needs a record weighing the dialect.

**Open**
- Whether SolEngine needs it once systems hold no members at all. It would let members return as read-only configuration, which 0109 refused.
- What `const Foo@` returned from a `const` method means for a caller that wants to write through it.

**Read**
- [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) — the holes, each probed against 2.38.0
