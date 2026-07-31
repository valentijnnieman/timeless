#!/bin/bash
# Configure, build, and package a timeless game's WASM build for itch.io in one
# shot. Run from the GAME's repo root (it uses ./ as the CMake source and
# ./build/wasm as the build dir). Output: build/wasm/<exe-lowercased>.zip
#
# Defaults to the THREADED build (EMSCRIPTEN_PTHREADS=ON) so FMOD mixes on its
# own worker thread and a main-thread stall doesn't drop audio. The threaded
# build needs cross-origin isolation: enable itch.io's "SharedArrayBuffer
# support", and test locally with serve-wasm.py (NOT plain http.server).
#
# Usage (from the game repo root):
#   /path/to/timeless/web/build-wasm.sh            # threaded (default)
#   /path/to/timeless/web/build-wasm.sh --single   # single-threaded fallback
#   /path/to/timeless/web/build-wasm.sh --clean    # wipe build/wasm first (use
#                                                  # when switching threading mode)
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$PWD"
BUILD="$ROOT/build/wasm"
PTHREADS=ON
CLEAN=0

for arg in "$@"; do
  case "$arg" in
    --single)   PTHREADS=OFF ;;
    --threaded) PTHREADS=ON ;;
    --clean)    CLEAN=1 ;;
    *) echo "unknown option: $arg" >&2; exit 1 ;;
  esac
done

command -v emcmake >/dev/null 2>&1 || {
  echo "error: emcmake not found — source the emsdk first (e.g. 'source \$EMSDK/emsdk_env.sh')" >&2
  exit 1
}
[ -f "$ROOT/CMakeLists.txt" ] || {
  echo "error: no CMakeLists.txt in $ROOT — run this from the game's repo root" >&2
  exit 1
}

if [ "$CLEAN" = 1 ]; then
  echo "==> Cleaning $BUILD"
  rm -rf "$BUILD"
fi
mkdir -p "$BUILD"

echo "==> Configuring (EMSCRIPTEN_PTHREADS=$PTHREADS)"
emcmake cmake "$ROOT" -B "$BUILD" -DEMSCRIPTEN=ON -DEMSCRIPTEN_PTHREADS="$PTHREADS"

echo "==> Building"
# Force the link step to pick up shell.html changes (not tracked as a CMake dep).
rm -f "$BUILD"/*.html
emmake make -C "$BUILD" -j"$(nproc)"

echo "==> Packaging"
"$SCRIPT_DIR/package-wasm.sh"

if [ "$PTHREADS" = ON ]; then
  echo
  echo "Threaded build — remember to enable itch.io's \"SharedArrayBuffer support\"."
  echo "Test locally with: $SCRIPT_DIR/serve-wasm.py"
fi
