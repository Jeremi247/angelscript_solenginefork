# Changelog

Scope: what changes for SolEngine between releases of this fork, and what it does to take each change. Which commits differ from upstream is [SOLENGINE.md](../SOLENGINE.md)'s. Why each change was made is its record's.

**How it is kept.** A commit that changes what SolEngine vendors, builds or sees, or what the release check proves, adds its line under Unreleased, in the same commit. Cutting a release turns Unreleased into the tag's heading, with the upstream commit it sits on, and opens an empty Unreleased above it. Newest first.

## Unreleased

Upstream unchanged since `sol-2.39.0-wip.1`, at `c9e74722`.

**Vendoring this tree with no `AS_SOL_*` defined builds and behaves as upstream's.** Every feature below is off by default ([0002](decisions/0002-an-engine-feature-is-off-by-default.md)), and the release check runs upstream's suite with them off.

### Build

- **New files.** `include/angelscript_sol.h` holds the features' functions. `source/as_sol.h` and `source/as_sol.cpp` hold their code, and `as_sol.cpp` compiles to nothing with every feature off.
- **To turn on any feature**, add `as_sol.cpp` to the library's source list, and define the feature `PUBLIC`: `angelscript_sol.h` declares a feature's functions only where its definition reaches.

### Features

| Definition | What it gives | What SolEngine does to use it | Record |
|---|---|---|---|
| `AS_SOL_DECLARATION_POSITIONS` | `asSolGetTypeDeclaredAt`, `asSolGetPropertyDeclaredAt` and `asSolGetGlobalVarDeclaredAt` return the section, row and column of a script type, property or global. Only source has positions: a registered or loaded declaration returns `asNOT_SUPPORTED` | Read them in `ModuleSet::Install`'s refusals. Each property and global costs 8 bytes more | [0005](decisions/0005-a-declaration-reports-where-it-was-made.md) |
| `AS_SOL_CYCLE_CREATION_SITE` | `asSolGetObjectCreatedAt` returns the script line that created a collected object, for the circular reference callback | Define it only where source is compiled. Each collectable object costs 8 bytes more | [0006](decisions/0006-a-collected-object-says-where-it-was-created.md) |
| `AS_SOL_GC_REPORTS_DONE` | An incremental `GarbageCollect` returns 0 once it has nothing left to do, instead of always 1. The call after a 0 starts a new pass | Step until 0 or the frame's budget. A cycle can take more than one round to free | [0007](decisions/0007-a-collector-step-says-when-it-is-done.md) |
| `AS_SOL_DEEP_CONST` | A handle member read through a `const` object, or `this` in a `const` method, is a handle to `const`. **Changes the language**: a script writing through such a handle stops compiling | Nothing beyond the definition | [0008](decisions/0008-const-reaches-through-a-handle-member.md) |
| `AS_SOL_NO_SCRIPT_STATE` | `asSolAddStatelessInterface` refuses members on a class implementing the named interface. `asSolSetConstantGlobalsOnly` refuses globals that are not `const` POD. Each refusal is a compile error at the declaration. Only source is checked | Call both before building a module. The refusals in `ModuleSet::Install` can stay as a second line for loaded bytecode, or go | [0009](decisions/0009-a-module-can-be-told-to-hold-no-state.md) |
| `AS_SOL_SAFE_VALUE_INOUT` | With `asEP_ALLOW_UNSAFE_REFERENCES` off, a value type may be `&inout`. A property or global is passed as a copy and assigned back after the call | Turn the engine property off. `T &inout` stays a written component's spelling | [0010](decisions/0010-a-value-type-crosses-as-inout-safely.md) |
| `AS_SOL_SIZED_FREE` | `asSolSetGlobalMemoryFunctions` takes a free that is told the block's size. `asSolFreeMem` frees an `asAllocMem` block by size. **x64 Windows only**, MSVC and clang-cl: other platforms' calling convention files do not compile with it on | Call it before creating an engine, pass the size to `IAllocator::Deallocate`, and drop the 16-byte header | [0011](decisions/0011-a-free-is-told-the-size.md) |

### Release check

- **The features have their own tests**, `check/Features/`, run with every feature on beside upstream's suite ([0004](decisions/0004-a-feature-is-tested-beside-upstreams-suite.md)).
- **A tools-off row** builds the features with `AS_NO_COMPILER` and links a program against them. It proves the configuration links, not that it behaves ([testing](testing.md)).

## sol-2.39.0-wip.1 — 2026-10-09

At upstream `c9e74722` (2026-10-05). The first release. No commit of ours changes `sdk/`, so the vendored tree is upstream's.

- **Upstream's newest commit fixes a leak** when a `foreach` over objects is left by `break`.
- **The release check** builds and runs upstream's suite under SolEngine's flags, MSVC and clang-cl, Debug and Development ([testing](testing.md)).
