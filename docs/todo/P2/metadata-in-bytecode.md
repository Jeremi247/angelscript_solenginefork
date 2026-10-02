# Metadata that survives into bytecode

**Ask** — a declaration's attributes, such as `[System(Gameplay)]`, are stored in the module and saved with its bytecode, so a build without tools reads them too. Today metadata belongs to the `scriptbuilder` add-on and is lost when the build ends, which is why SolEngine declares a system by implementing an interface instead ([SolEngine 0074](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0074-a-script-system-is-a-class-implementing-a-phase-interface.md)).

**Open**
- Whether SolEngine wants it. 0074 found the interface better on its own terms: an interface cannot hold state. Nothing in the engine is waiting on metadata.
- The bytecode format change, which every saved module then depends on.

**Read**
- [SolEngine 0074](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0074-a-script-system-is-a-class-implementing-a-phase-interface.md) — why metadata was ruled out, and what replaced it
- `sdk/add_on/scriptbuilder/` — where metadata lives today
