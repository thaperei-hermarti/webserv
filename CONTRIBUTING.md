# Contributing to Webserv

Thanks for your interest in contributing! This document explains how to set up the project, what conventions we follow, and how to get your changes reviewed and merged.

## Getting started

Run `make setup` once after cloning. It checks for missing dependencies, installs whatever is needed, and then installs the git hooks (`pre-commit`, `commit-msg`):

```sh
git clone git@github.com:<you>/webserv.git
cd webserv
make setup
```

### Dependencies

| Dependency      | Used for                              | Install
|-----------------|---------------------------------------|-------------------
| C++ compiler    | building the server (`-std=c++98`)    | system (clang/g++)
| clang-format    | `make format` / `format-check`        | auto-installed
| clang-tidy      | `make lint`                           | auto-installed
| GoogleTest      | `make test-run` (unit tests)          | auto-installed
| valgrind        | `make test-run-valgrind`              | auto-installed
| cmake, git      | building GoogleTest from source       | system

Missing tools are installed automatically — no sudo required. The installer tries, in order:

1. **brew** (user-level, no sudo; also searched outside PATH, e.g. `~/.brew`, `/goinfre/brew`)
2. **apt + sudo** (only if sudo actually works)
3. **pip --user** (clang-format/clang-tidy wheels) + GoogleTest built from source into `~/.local`

If pip was used, add `~/.local/bin` to your `PATH` (the installer checks it even when absent from PATH):

```sh
export PATH="$HOME/.local/bin:$PATH"
```

A GoogleTest built into `~/.local` is auto-detected by the build (no extra setup).

The hooks are mandatory: they enforce formatting, linting, a clean build, and commit message conventions on every commit. `make check-tools` will tell you if anything is missing.

## Project layout

```
inc/        public headers
src/        source files (http/, parser/, config/, server/, cgi/)
test/       GoogleTest unit tests (mirrors the src/ tree)
config/     example server configuration files
scripts/    helper scripts and git hooks
```

## Development workflow

We work on the `dev` branch. All changes land via pull requests:

1. Create a branch off `dev` with a descriptive name (e.g. `feat/cgi-timeout`, `fix/chunked-parser`).
2. Make your changes, keeping each PR focused on a single concern.
3. Run the checks below before committing.
4. Open a pull request against `dev` and wait for review.

### Building

```sh
make            # build the server
make DEBUG=1    # build with debug symbols (-g2 -O0)
```

### Checking your work

```sh
make format         # auto-format all sources with clang-format
make format-check   # verify formatting without modifying files
make lint           # run clang-tidy
make test-run       # build and run the GoogleTest suite
make test-run-valgrind  # run tests under valgrind (leaks, fds)
```

The `pre-commit` hook runs `format-check`, `lint`, and a full rebuild automatically. Run `make format` first if it complains about formatting, then re-stage your files.

### Adding tests

New behaviour must come with unit tests in `test/unit/`, mirroring the directory layout of `src/` (e.g. `test/unit/server/router_test.cpp` tests `src/server/Router.cpp`). Run the full suite with `make test-run` to make sure nothing regressed.

## Code style

- Follow the existing code style; `.clang-format` and `.clang-tidy` at the repo root are authoritative.
- `make format` is the only accepted way to fix formatting, and `make format-check` must pass.
- The production code is compiled with `-std=c++98` (`make lint` and the hooks enforce this), so no C++11+ features in `src/` or `inc/`. Tests may use C++17.
- Keep the 42 school header in every source file you touch, matching the existing files.
- No dead code, no commented-out code, no unused variables — `-Wall -Wextra -Werror` is on.

## Commit conventions

Commits must follow [Conventional Commits](https://www.conventionalcommits.org), enforced by the `commit-msg` hook:

```
<type>(<optional scope>): <description>
```

Types: `build`, `chore`, `ci`, `docs`, `feat`, `fix`, `perf`, `refactor`, `revert`, `style`, `test`.

Examples:

```
feat(parser): handle chunked request bodies
fix(cgi): reap zombie children without blocking the loop
test(session): add expiry edge case
```

For breaking changes, add `!` before the colon (`feat!: ...`) or a `BREAKING CHANGE` footer.

## Pull requests

- Target the `dev` branch.
- Keep PRs small and reviewable; split large changes into multiple PRs.
- Describe what the change does and why, and mention any tests you added.
- Make sure the checks in the GitHub Actions workflow pass before requesting a review.

## Getting help

If you're unsure about anything, open an issue or reach out before writing a lot of code — it's easier to align on design early than to rework a large PR.