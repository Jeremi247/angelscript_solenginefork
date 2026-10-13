# 0005 — A declaration reports where it was made

Scope: `AS_SOL_DECLARATION_POSITIONS`, the feature that lets SolEngine print its load-time refusals at a `file:line` ([SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md)). Whether the compiler should refuse the same declarations itself is [0009](0009-a-module-can-be-told-to-hold-no-state.md)'s.

## The decision

**A script type, a property of a type and a global variable of a module report the section, row and column of their declared name.** Three functions in `angelscript_sol.h`:

```cpp
int asSolGetTypeDeclaredAt(const asITypeInfo *type, const char **scriptSection, int *row, int *col);
int asSolGetPropertyDeclaredAt(const asITypeInfo *type, asUINT index, const char **scriptSection, int *row, int *col);
int asSolGetGlobalVarDeclaredAt(const asIScriptModule *module, asUINT index, const char **scriptSection, int *row, int *col);
```

- **Types are classes, interfaces, enums, funcdefs and typedefs.** A template instance reports where it was first used, which upstream already records.
- **An inherited property reports where its base class declared it.**
- **Only source has positions.** A registered declaration, or one loaded from bytecode, returns `asNOT_SUPPORTED`, as `asIScriptFunction::GetDeclaredAt` does for a function without one.
- **A shared type keeps the position of the module that first declared it.**

## Why

**The position is free at build time and gone after it.** The builder holds each declaration's node and script section while it registers it, and frees both when the build ends. A function keeps its position in its script data. Types already carry fields for one, which upstream fills only for template instances. Properties and globals gain eight bytes each.

**Source only, because only source is refused.** A build without tools loads bytecode the cook already refused or accepted ([SolEngine 0073](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0073-a-failed-compile-refuses-the-module.md)). Saving positions in bytecode would change the format every saved module depends on, for refusals that cannot happen there.

**Behind a definition, though upstream might take it.** It is a plain addition, so it could be offered upstream ([0002](0002-an-engine-feature-is-off-by-default.md)). Until upstream has it, it is ours, and gated like any other feature.

## Consequences

- **SolEngine's refusals in `ModuleSet::Install` can read `Movement.as:3:5: ...`**, in the compiler's format. Its refusals about a system's `Tick` could already, through `GetDeclaredAt`.
- **Five tests hold it** in `check/Features/DeclarationPositions.cpp`: each kind of type, own and inherited properties, globals declared together, and a registered type with no position.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) — the refusals that wanted a position
