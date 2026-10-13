# 0010 — A value type crosses as `&inout` safely

Scope: `AS_SOL_SAFE_VALUE_INOUT`, which lets a registered value type be declared `&inout` with `asEP_ALLOW_UNSAFE_REFERENCES` off. SolEngine turned that property on for the whole engine to spell a written component ([SolEngine 0083](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0083-a-written-component-crosses-as-inout.md)), and owes a re-read of 0083 before it registers its first reference type. Turning the property off is SolEngine's.

## The decision

**With unsafe references off, a value type may be declared `&inout`**, by a script function or a registered one. What an argument costs depends on whether the compiler knows it outlives the call:

- **Passed by address**: a local variable, a parameter (a reference parameter included), a temporary, and anything else the compiler already treats as safe, such as a value member of a local.
- **Passed as a copy, assigned back after the call**: any other argument. It must be a variable followed by property accesses, such as `b.P` or `Global.P.Q`, so evaluating it again has no side effects.
- **Refused**, with "A value type passed to '&inout' must be a local variable, a parameter or a property": any other expression, such as `MakeBox().P` or a conditional.
- **A `const &inout` takes a copy of any expression**, and assigns nothing back.

**With unsafe references on, nothing changes.** Arguments are passed by address, as upstream passes them.

## Why

**The property allows a write to freed memory.** With it on, this compiles, and `Kill` writes into the `Box` it just released:

```angelscript
Box@ Held;
void Kill(FPosition &inout P) { @Held = null; P.X = 1; }
@Held = Box(); Kill(Held.P);
```

Under the feature, `P` is a copy. The write lands in it, and assigning it back to `Held.P` raises a null pointer exception. The fault becomes a script exception.

**A local cannot die during the call, so the copy is only for what can.** Script cannot reach a caller's local except through the reference it was given. A parameter refers to something that outlives the call it was passed to, because every argument either outlives the call or is a copy. Upstream's own rule for `const &in` draws the same line.

**Assigning back means evaluating the argument twice.** `&out` already compiles its argument after the call and assigns to it. The copy reuses that path, and limiting it to property chains keeps the second evaluation free of side effects. An index or a call could be allowed later. Both are refused today, since `opIndex` is a call and SolEngine registers no container yet.

**It accepts only what safe mode refused.** A script valid upstream with the property off compiles to the same code, so no meaning changes.

## Consequences

- **SolEngine can turn the property off** and keep `T &inout` as a written component's spelling. A dispatcher calling from C++ passes its own local, which is safe by construction.
- **Aliasing becomes copy-in, copy-out for the copied case.** A callee that reads the original through another path sees the value from before the call. Aliasing between locals is unchanged.
- **A template's subtype rules are unchanged.** A template method taking `T &inout` still refuses a value subtype in safe mode.
- **Nine tests hold it** in `check/Features/SafeValueInout.cpp`: declaration in safe mode, locals and parameters by address, properties and globals assigned back, the fault above, the refusals, `const &inout`, unsafe mode left alone, and the copy not aliasing.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [SolEngine 0083](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0083-a-written-component-crosses-as-inout.md) — the property, and the trigger to re-read it
- [SolEngine 0085](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0085-the-dispatcher-refuses-at-registration.md) — the aliasing bounded at the dispatch entry
