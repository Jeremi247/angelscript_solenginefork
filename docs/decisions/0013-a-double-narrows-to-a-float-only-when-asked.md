# 0013 — A double narrows to a float only when asked

Scope: `AS_SOL_DOUBLE_NARROWING_WARNS`, a change to what the compiler says rather than to the interface: an implicit conversion of a `double` to a `float`. SolEngine places the world in doubles, and every vertex, constant and solver input is a float, so this is where a position passes silently through a float ([SolEngine's large-world assumption](https://github.com/Jeremi247/SolEngine/blob/main/docs/architecture/critical-assumptions.md#a-world-is-larger-than-a-float-can-place)). What a warning does to a module is the application's: SolEngine refuses it.

## The decision

**A `double` converted implicitly to a `float` is a compiler warning.** No new function.

- **A variable or any other expression warns** "Double value narrowed in implicit conversion to float", at the expression.
- **A constant warns when a float does not hold it exactly**, with upstream's "Implicit conversion of value is not exact", as upstream already warns for an integer constant. `0.5` and `1.0e10` are quiet; `0.1` and `1.0e11` warn.
- **Every implicit narrowing warns**: an initialisation, an assignment, a compound assignment, an argument and a return. A mixed `float * double` is a `double`, so storing it warns too.
- **An explicit conversion does not**: `float(d)` is the spelling for meaning it.
- **Overload resolution is unchanged.** A narrowing still costs what it cost, so a call picks the overload it picked; the warning is emitted only for the conversion the compiler generates.

## Why

**Upstream narrows a double to a float without a word**, a constant or a variable: `ImplicitConversionConstant` converts and stores, and the variable path emits `asBC_dTOf`. Every other narrowing it can judge already warns: a float truncating to an int, a constant changing sign, and an integer constant a float cannot hold exactly. So the gap is this one conversion, and the constant half reuses the integer constants' message and test.

**A warning, not an error**, as the library's other conversion diagnostics are. The application decides what a warning costs through `asEP_COMPILER_WARNINGS`, and SolEngine already makes every warning refuse the module ([SolEngine 0211](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0211-a-script-warning-refuses-the-module.md)). C++ compilers make the same call: MSVC's C4244 and clang's `-Wfloat-conversion` are warnings that a build promotes.

**Not a change to overload costs.** Making narrowing no conversion at all, C#'s rule, would change which overload a mixed call picks, which the warning does not. SolEngine 0211 left that open, and this does not decide it.

## Consequences

- **A script that narrows a double without saying so warns**, so under SolEngine's engine property it does not compile. One that spells `float(d)` compiles as before.
- **Six tests hold it** in `check/Features/DoubleNarrowingWarns.cpp`: the option is reported, each implicit narrowing of a variable warns, once, an inexact constant warns as not exact, an explicit conversion, a widening and an exact constant are quiet, and the converted value is upstream's. With the definition removed, the first, second, third and fourth fail.

## See also
- [0004-a-feature-is-tested-beside-upstreams-suite](0004-a-feature-is-tested-beside-upstreams-suite.md) — where those tests run
- [0002-an-engine-feature-is-off-by-default](0002-an-engine-feature-is-off-by-default.md) — why a diagnostic is a feature
- [SolEngine 0211](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0211-a-script-warning-refuses-the-module.md) — what a warning does to SolEngine's module
