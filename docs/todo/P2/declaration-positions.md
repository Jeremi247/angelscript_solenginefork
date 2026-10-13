# Where a type, a property and a global were declared

**Ask** — `asITypeInfo`, its properties and a module's globals report the section, row and column they were declared at, as `asIScriptFunction::GetDeclaredAt` already does for a function. An API addition, not a language change.

SolEngine refuses a stateful system and a writable global after the build, in `ModuleSet::Install` ([SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md)). The compiler's own errors read `Movement.as:3:5: ...`; these read `Script class 'MoveSystem' declares member 'int Count'`, with nothing to jump to. With positions, SolEngine prints its refusals in the compiler's format.

**Not this item** — the refusals about a system's `Tick` can name a position today through `GetDeclaredAt`, and SolEngine does not call it yet. That is SolEngine's to fix.

**Open**
- Whether the compiler already keeps these positions after the build, or whether keeping them costs memory per declaration.
- Whether a position survives bytecode. A build without tools never prints these refusals, since the cook refused the module first ([SolEngine 0073](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0073-a-failed-compile-refuses-the-module.md)), so source only may be enough.
- Gated behind `AS_SOL_` or offered upstream. It is a plain addition upstream may take ([0002](../../decisions/0002-an-engine-feature-is-off-by-default.md)).

**Read**
- [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) — the refusals that want a position
- [A system class the compiler knows](compiler-known-system-class.md) — the language-change route to the same `file:line`
- `sdk/angelscript/include/angelscript.h` — `GetDeclaredAt`, the shape to match
- `sdk/angelscript/source/as_scriptfunction.cpp` — where a function keeps its position
