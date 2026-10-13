# 0009 — A module can be told to hold no state

Scope: `AS_SOL_NO_SCRIPT_STATE`, which moves [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md)'s two refusals into the compiler: a system class declares no members, and a global is a constant of a plain value. Whether SolEngine keeps its own check is SolEngine's.

## The decision

**The application names the interfaces that mark a stateless class, and switches on the rule for globals.** Two functions in `angelscript_sol.h`, called on the engine before a build:

```cpp
int asSolAddStatelessInterface(asIScriptEngine *engine, const char *name);
int asSolSetConstantGlobalsOnly(asIScriptEngine *engine, bool on);
```

- **A script class implementing a named interface may declare no members**, inherited ones included. Implementing it through a base class or another interface counts. A name in a namespace is written qualified, as `Game::IRenderSystem`.
- **With the rule on, a global must be `const`**, and of a primitive, an enum, or a value type registered `asOBJ_POD`. A handle, a script object and a value type with behaviours are refused, `const` or not.
- **Each refusal is an ordinary compile error at the declaration.** A member is reported where it was declared, even in a base class, and the error names the class, the interface and the member.
- **Only source is checked.** Bytecode that is loaded is not.
- **Nothing is refused until the application asks**, so a build with the feature on behaves as upstream's until then.

## Why

**The phase interfaces are SolEngine's, so the library has to be told them.** They are declared in a prelude section of every module ([SolEngine 0074](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0074-a-script-system-is-a-class-implementing-a-phase-interface.md)), and not shared, so each module has its own type of the same name. A name is what is the same across modules. An engine property cannot carry a list of names, so these are two functions.

**A dialect only where it is switched on.** The rules refuse code upstream accepts, which makes them a language change ([0002](0002-an-engine-feature-is-off-by-default.md)). An application that names no interface, and leaves the rule for globals off, compiles exactly what upstream compiles.

**In the compiler, because an error there has a position.** SolEngine's check after the build names a class and a member and nothing to jump to. The compiler holds the declaration's node while it builds, so the error reads like every other. [0005](0005-a-declaration-reports-where-it-was-made.md) gave SolEngine's own check a position through the interface. This one makes the refusal happen where every other compile error does.

**Not for bytecode, because bytecode was compiled.** A build without tools loads what the cook accepted, and the cook is the same engine with the same rules ([SolEngine 0072](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0072-the-game-process-compiles-its-own-script.md)).

**The plain value is 0109's rule, read off registration.** `asOBJ_POD` is how the application says a value type has no constructor, destructor or copy behaviour. That is the property 0109 relies on, and the library already records it.

## Consequences

- **SolEngine's refusals in `ModuleSet::Install` can stay as a second line for loaded bytecode, or go.** Either way, a source module with a stateful system now fails its build at a `file:line`.
- **Seven tests hold it** in `check/Features/NoScriptState.cpp`: own and inherited members, interface inheritance, qualified names, classes and builds the rule leaves alone, the globals accepted, and each kind of global refused.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [0005-a-declaration-reports-where-it-was-made](0005-a-declaration-reports-where-it-was-made.md) — the interface route to the same position
- [0008-const-reaches-through-a-handle-member](0008-const-reaches-through-a-handle-member.md) — the `const` holes that made 0109 refuse members outright
- [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) — the rule this enforces
