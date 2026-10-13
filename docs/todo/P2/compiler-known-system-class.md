# A system class the compiler knows

**Ask** — the compiler refuses a class implementing a SolEngine phase interface that declares a member, and refuses a global that is not a constant of a plain value, each as an ordinary compile error at its `file:line`. SolEngine does this today after the build, in `ModuleSet::Install`, naming the class and the member but no position ([SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md)).

**Open**
- How the compiler knows a class is a system. The phase interfaces are SolEngine's, declared in a prelude, so the library needs a way to be told which interfaces mark one, such as an engine property naming them.
- Whether the load-time check in SolEngine stays as a second line of defence for bytecode compiled elsewhere.
- Whether this is worth a language change at all, when the load-time check already refuses the same modules and [declaration positions](../../decisions/0005-a-declaration-reports-where-it-was-made.md) give it a `file:line` through the API.

**Read**
- [SolEngine 0109](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0109-a-dispatch-can-reach-no-script-state.md) — the rule, and the `const` holes that made it a refusal
- [SolEngine 0074](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0074-a-script-system-is-a-class-implementing-a-phase-interface.md) — what makes a class a system
