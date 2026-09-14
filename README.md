# FastCalc

FastCalc is an extremely lightweight, fast, responsive graphical calculator
for Linux. Tiny native utility: C17, SDL3 for window/input/rendering,
FreeType for text, nothing else.

## Dependencies

- C17 compiler (gcc or clang)
- CMake >= 3.22
- SDL3 development files (`libsdl3-dev` on Ubuntu)
- FreeType development files (`libfreetype-dev` on Ubuntu)
- A system TrueType font (DejaVu Sans is used if present)

Install on Ubuntu/Debian:

```bash
sudo apt install build-essential cmake libsdl3-dev libfreetype-dev fonts-dejavu-core
```

## Build

Quick build (configures, compiles and runs the unit tests):

```bash
./build.sh
```

Useful options: `./build.sh --clean` (wipe build dirs first),
`./build.sh --sanitize` (ASan+UBSan into `build-san/`),
`./build.sh --debug`, `./build.sh -j N`, `./build.sh --help`.

Or with plain CMake:

```bash
cmake -S . -B build
cmake --build build -j
```

Output: `build/fastcalc`

Strict warnings (`-Wall -Wextra -Wpedantic -Wconversion -Wshadow`) are
always on. For development with sanitizers:

```bash
cmake -S . -B build-san -DSANITIZE=ON
cmake --build build-san -j
```

Run the unit tests:

```bash
ctest --test-dir build --output-on-failure
```

## Run

```bash
./build/fastcalc
```

## Install (Ubuntu)

Builds (Release) and installs system-wide under `/usr/local`
(binary, app launcher and icon). Prompts for sudo only if needed:

```bash
./install.sh
```

Then launch with `fastcalc` or from the app grid.

Useful options: `./install.sh --deps` (install build dependencies via
apt first), `./install.sh --prefix DIR` (e.g. `--prefix ~/.local` for a
user install without sudo), `./install.sh --no-build`, `./install.sh
--uninstall`, `./install.sh --help`.

## Keyboard shortcuts

| Keys | Action |
|---|---|
| `0-9` | Enter digits |
| `+ - * /` | Operators |
| `Enter` / `=` | Equals |
| `.` / `,` | Decimal point |
| `Backspace` | Delete last digit |
| `Esc` / `Del` / `C` | Clear |
| `%` | Percentage |
| `Ctrl+C` / `Ctrl+V` | Copy display / paste number |
| `Ctrl+Plus` / `Ctrl+Minus` / `Ctrl+0` | Bigger / smaller / reset font |
| `Ctrl+T` or `F10` | Toggle dark/light theme |

Keypad keys work the same as their main-keyboard equivalents.

## Architecture

```
src/
  main.c          Process entry: init -> run -> shutdown.
  app.c/h         Central App state; SDL/window/renderer/font lifecycle;
                  button dispatch; font-size and theme controls.
  calculator.c/h  UI-independent engine (state, operators, formatting).
                  Unit-testable without SDL.
  renderer.c/h    Deterministic event-driven paint (no idle redraws).
  input.c/h       Keyboard + mouse routing, hover/pressed state.
                  Printable keys are read from SDL text-input events, so
                  every layout yields the real character (Shift+'=' gives
                  '+', keypad digits work with NumLock, ...). Keydown
                  handles control keys, keypad operators, Ctrl shortcuts.
  font.c/h        FreeType face + text measurement/drawing.
  glyph_cache.c/h Fixed MRU texture cache: each glyph rasterized once.
  layout.c/h      Button grid as data; rects computed from window size.
  clipboard.c/h   Copy display / paste numeric text via SDL clipboard.
  theme.c/h       Single dark + light palette, no theme engine.
```

Main loop: block in `SDL_WaitEvent`, drain queued events, re-render only
when state changed. No threads, no polling, ~zero idle CPU.

Number formatting policy (`format_number`): `%.15g` removes trailing zeros
and FP artifacts (`0.1 + 0.2` shows `0.3`); negative zero shows as `0`;
non-finite results show `Error`; huge/tiny magnitudes use `%g` scientific
notation instead of overflowing.

Percentage behavior: standalone `50 %` -> `0.5`; with `+`/`-` pending,
`200 + 10 %` -> `220` (percentage of the left operand); with `x`/`/`
pending, plain hundredths.

## Performance benchmark (manual)

- **Startup:** `time ./build/fastcalc` to a usable window (target: instant,
  no filesystem scans, one font, lazy glyph rasterization).
- **Memory:** RSS shortly after startup, e.g. `ps -o rss= -p <pid>`.
- **Idle CPU:** leave idle 30 s, observe with `top` (target: 0 %, the loop
  blocks in `SDL_WaitEvent`).
- **Interaction:** key/mouse event -> state update -> single repaint; no
  allocations in the render path besides first-time glyph uploads.

## License

MIT — see [LICENSE](LICENSE).
