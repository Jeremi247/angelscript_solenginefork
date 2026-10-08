# 0002 — An engine feature is off by default

Scope: how something SolEngine needs and upstream does not have enters the library. A *fix* to upstream's own behaviour is not a feature, and is not behind anything. Which features exist is each one's own record.

## The decision

**A feature is behind a compile definition named `AS_SOL_<FEATURE>`, and with it undefined the library behaves exactly as upstream's.** SolEngine turns a feature on in its own build, as a compile definition in `ThirdParty/AngelScript/CMakeLists.txt`, which is how it already configures the library ([SolEngine 0076](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0076-vendoring-a-third-party.md)).

**`asGetLibraryOptions()` reports every `AS_SOL_` definition that is on**, beside upstream's own options, so a SolEngine test can assert which features a build actually has.

**A feature brings its own tests**, built and run with it on. Upstream's suite runs with every feature off.

**A fix is not gated.** A change that makes upstream's behaviour correct, such as a null check upstream lacks, applies always and is reported upstream.

## Why

**Upstream's suite has to keep testing upstream's library.** It is the release check that replaces upstream's own testing before a tag ([SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)). With a feature always on, an upstream test that fails could be upstream's bug or ours, and the check would stop saying which. Off by default keeps one question per run: does the library still do what upstream's does, then does our feature do what it says.

**A feature can be read as a diff.** Everything a feature adds is inside its `#if`, so what SolEngine changed is a search for `AS_SOL_`. A rebase conflict inside one is ours to resolve, and one outside every guard is a fix's.

**The guard is how the library already works.** Upstream configures itself through `AS_*` definitions (`AS_NO_COMPILER`, `AS_NO_EXCEPTIONS`, `AS_MAX_PORTABILITY`), and SolEngine already sets them from its build. A feature is one more of those.

## What it costs

- **Two configurations of every feature's code**, compiled and tested both ways: upstream's suite covers the off side, a feature's tests the on side.
- **A guard inside a hot or central function is a conflict at every rebase that touches it.** [0003](0003-a-commit-of-ours-keeps-upstreams-shape.md) says how to keep that rare.

## Consequences

- **Where a feature's tests live, and how they are built, is decided with the first feature.** Upstream's suite is built by `check/` ([testing](../testing.md)), the toolchain the two will share.
- **A feature SolEngine turns on is listed in [SOLENGINE.md](../../SOLENGINE.md)'s release table**, so a release says which of its features the engine uses.

## See also
- [0001-the-fork-follows-upstream](0001-the-fork-follows-upstream.md) — the branch a feature is committed to
- [0003-a-commit-of-ours-keeps-upstreams-shape](0003-a-commit-of-ours-keeps-upstreams-shape.md) — how it is written
- [Testing](../testing.md) — the two runs
