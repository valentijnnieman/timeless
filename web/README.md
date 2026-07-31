# timeless/web — WebAssembly (Emscripten) tooling

Reusable shell + build scripts for shipping a timeless game to the web
(itch.io). Run the scripts **from your game's repo root** — they operate on
`./` (CMake source) and `./build/wasm` (build dir) and auto-detect the
Emscripten target's name.

```bash
# from <game-repo>/
timeless/web/build-wasm.sh           # configure + build + package (threaded)
timeless/web/build-wasm.sh --single  # single-threaded fallback
timeless/web/build-wasm.sh --clean   # wipe build/wasm first (switching modes)
timeless/web/serve-wasm.py           # local test server (COOP/COEP headers)
timeless/web/package-wasm.sh         # (re)package build/wasm/<exe>.zip only
```

Output is `build/wasm/<exe-lowercased>.zip` containing `index.html`
(renamed from `<Exe>.html`), `<Exe>.js`, `<Exe>.wasm` — upload that to itch.

## shell.html

`shell.html` is wired in automatically: timeless adds `--shell-file
.../web/shell.html` as a PUBLIC link option for the Emscripten build, so any
target linking timeless inherits it. To use your own branded shell, point
`--shell-file` at your copy (the last `--shell-file` wins). It provides:

- **Letterboxed canvas scaling** for timeless's fixed-resolution rendering, incl.
  scaling *up* to fill fullscreen.
- **Native fullscreen** (the ⛶ button) that doesn't fight the scaling.
- **Audio start-gate** (see below).

## Audio start-gate contract

Browsers won't play audio until a user gesture *in the game's own document*
(itch's "Run game" button is in the parent page and doesn't count), and FMOD's
`AudioContext.resume()` must run synchronously inside that gesture. The shell
shows a "Click to start" overlay, waits until FMOD's context exists, then on the
click resumes it in-handler and sets `window._timelessStarted = true`.

**Your game's main loop must freeze the simulation until that flag is set**, so
the intro/audio start together. Reference implementation (Beach `app.hpp`):

```cpp
bool run_sim = true;
#ifdef __EMSCRIPTEN__
  static bool started = false;
  if (!started)
    started = emscripten_run_script_int("(window._timelessStarted?1:0)") != 0;
  run_sim = started;
#endif
if (run_sim) { /* update scene / cutscene / animation / sound */ }
// render every frame regardless
```

## Threaded build & itch.io

The default build is threaded (`-DEMSCRIPTEN_PTHREADS=ON`) so FMOD mixes on a
worker thread and main-thread stalls don't drop audio. SharedArrayBuffer
requires cross-origin isolation: **enable itch.io's "SharedArrayBuffer
support"**, and locally use `serve-wasm.py` (plain `http.server` sends no
COOP/COEP headers, so SAB is undefined and the threaded build won't start).
