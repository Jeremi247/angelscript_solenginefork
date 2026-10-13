# AngelScript, SolEngine's fork

AngelScript, forked for [SolEngine](https://github.com/Jeremi247/SolEngine), which vendors `sdk/angelscript/` from a release tag of this repository ([SolEngine 0110](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0110-angelscript-is-vendored-from-our-fork.md)). Upstream is [anjo76/angelscript](https://github.com/anjo76/angelscript). This file is only how to work in it.

## Before you touch anything

Read the doc that owns what you are changing. It holds the rule; this file does not repeat it.

| About to | Read first |
|---|---|
| Start any work | [todo](docs/todo.md), and [P0](docs/todo/P0.md) |
| Change anything under `sdk/` | [0003](docs/decisions/0003-a-commit-of-ours-keeps-upstreams-shape.md) |
| Add something SolEngine needs and upstream does not have | [0002](docs/decisions/0002-an-engine-feature-is-off-by-default.md) |
| Take upstream's changes, or cut a release | [0001](docs/decisions/0001-the-fork-follows-upstream.md), [testing](docs/testing.md), [changelog](docs/changelog.md) |
| Decide something structural | [the register](docs/decisions/README.md) |
| Write a doc, a record or a todo item | SolEngine's [docs & comments](https://github.com/Jeremi247/SolEngine/blob/main/docs/conventions/docs-and-comments.md) and [todo rules](https://github.com/Jeremi247/SolEngine/blob/main/docs/todo.md#how-these-lists-are-kept), which hold here unchanged |

## How you work

- **`sdk/` is upstream's code, in upstream's style.** SolEngine's C++ style is for code in `sol::`, and none of this is.
- **Commit on `sol/main`. `master` only ever mirrors upstream.**
- **Every commit that changes `sdk/` is listed in [SOLENGINE.md](SOLENGINE.md)**, in the same commit.
- **Every commit that changes what SolEngine vendors, builds or sees, or what the release check proves, gets a line in the [changelog](docs/changelog.md)'s Unreleased**, in the same commit.
- **Load narrowly.** Read the row above that fits the task, and nothing more.
- **A response states the finding, the evidence, and what it costs.** No preamble, and no recap.

## Commands

Take upstream's changes:

```bash
git fetch upstream
git checkout master
git merge --ff-only upstream/master
git checkout sol/main
git rebase master
```

Build and run upstream's suite the way SolEngine builds the library, MSVC and clang-cl, Debug and Development ([testing](docs/testing.md)):

```bash
powershell -ExecutionPolicy Bypass -File check/Check.ps1
```

## Where things are

[todo](docs/todo.md) · [decisions](docs/decisions/README.md) · [testing](docs/testing.md) · [what differs from upstream](SOLENGINE.md) · [changelog](docs/changelog.md) · the release check's build is `check/` · upstream's own manual is `sdk/docs/`
