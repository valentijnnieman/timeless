#!/bin/bash
# Package a timeless game's WASM build for itch.io.
# Run from the GAME's repo root (it operates on ./build/wasm). itch.io requires
# the entry page to be named index.html and serves the zip's contents directly,
# so we rename <Exe>.html -> index.html and bundle it with the JS loader and the
# .wasm module. Output: build/wasm/<exe-lowercased>.zip
#
# Usage:  cd <game-repo> && /path/to/timeless/web/package-wasm.sh [ExeName]
# ExeName is auto-detected from the single *.html Emscripten emitted if omitted.
set -e

BUILD="$PWD/build/wasm"
[ -d "$BUILD" ] || { echo "error: $BUILD not found — build first (see build-wasm.sh)" >&2; exit 1; }

# Determine the Emscripten target's base name (e.g. "Beach" -> Beach.html/.js/.wasm).
if [ -n "$1" ]; then
  NAME="$1"
else
  htmls=("$BUILD"/*.html)
  if [ ! -e "${htmls[0]}" ]; then
    echo "error: no *.html in $BUILD — build first" >&2; exit 1
  elif [ "${#htmls[@]}" -gt 1 ]; then
    echo "error: multiple *.html in $BUILD; pass the exe name explicitly" >&2; exit 1
  fi
  NAME="$(basename "${htmls[0]}" .html)"
fi

for f in "$NAME.html" "$NAME.js" "$NAME.wasm"; do
  [ -f "$BUILD/$f" ] || { echo "error: $BUILD/$f not found — build first" >&2; exit 1; }
done

OUT="$BUILD/$(echo "$NAME" | tr '[:upper:]' '[:lower:]').zip"

# Stage in a temp dir so <Exe>.html can be renamed to index.html without touching
# the build outputs (zip has no in-place rename).
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
cp "$BUILD/$NAME.html" "$STAGE/index.html"
cp "$BUILD/$NAME.js"   "$STAGE/$NAME.js"
cp "$BUILD/$NAME.wasm" "$STAGE/$NAME.wasm"

rm -f "$OUT"
( cd "$STAGE" && zip -q "$OUT" index.html "$NAME.js" "$NAME.wasm" )

echo "Done: $OUT"
unzip -l "$OUT"
