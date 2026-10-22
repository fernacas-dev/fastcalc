# Contributing to FastCalc

Thanks for your interest. FastCalc is intentionally small — please keep
changes focused and native (C17 + SDL3 + FreeType only).

## Workflow

1. Fork + branch from `master` (`feat/...`, `fix/...`).
2. Build + test before pushing:
   ```bash
   ./build.sh
   ```
3. For memory/UB-sensitive changes also run:
   ```bash
   ./build.sh --clean --sanitize
   ```
4. Open a PR with: what changed, why, manual test (key/mouse path),
   and `ctest` output.

## Code style

- C17, no extensions (`CMAKE_C_EXTENSIONS OFF`).
- Strict warnings are on: `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`
  — new code must compile warning-free with gcc and clang.
- Calculator logic stays UI-independent in `src/calculator.c`
  (no SDL includes). UI glue goes in `app.c` / `input.c` / `renderer.c`.
- Bounded buffers, no unchecked `strcpy`/`sprintf`. Prefer `snprintf`.
- No new dependencies without prior discussion in an issue.

## Tests

- Engine tests live in `tests/calculator_tests.c` (no SDL).
- Add a case for every new calculator behavior (formatting, `%`,
  error recovery, paste validation).
- GUI behavior (rendering, fonts) is verified manually for now.

## Commit messages

Short imperative summary + context when needed:

```
Fix percent-after-equals chaining

200 + 10 % = 220, then % again stays in hundredths...
```
