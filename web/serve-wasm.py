#!/usr/bin/env python3
"""Serve a timeless game's WASM build locally with the cross-origin isolation
headers that SharedArrayBuffer (and therefore the pthreads/threaded-FMOD build)
requires. Run from the GAME's repo root (it serves ./build/wasm).

Plain `python3 -m http.server` does NOT send these headers, so the threaded
build will fail to start under it (SharedArrayBuffer is undefined). itch.io's
"SharedArrayBuffer support" checkbox sets the same headers in production.

Usage:  cd <game-repo> && /path/to/timeless/web/serve-wasm.py [port]
        then open the printed http://localhost:<port>/<Exe>.html URL
"""
import glob
import http.server
import os
import socketserver
import sys

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
DIRECTORY = "build/wasm"


class COIHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


entry = sorted(os.path.basename(p) for p in glob.glob(f"{DIRECTORY}/*.html"))
entry_name = entry[0] if entry else "index.html"

with socketserver.TCPServer(("", PORT), COIHandler) as httpd:
    print(f"Serving {DIRECTORY}/ with COOP+COEP at http://localhost:{PORT}/{entry_name}")
    print("In the page console, self.crossOriginIsolated should be true.")
    httpd.serve_forever()
