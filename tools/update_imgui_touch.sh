#!/usr/bin/env bash
# Refreshes the vendored copy of imgui_touch (external/imgui_touch/imgui_touch) from a checkout of
# https://github.com/pthom/imgui_touch: its header and sources only (no tests, no CI).
# Usage: tools/update_imgui_touch.sh <path to an imgui_touch checkout>
set -euo pipefail
src=${1:?"usage: $0 <path to an imgui_touch checkout>"}
here=$(cd "$(dirname "$0")/.." && pwd)
dest="$here/external/imgui_touch/imgui_touch"
rm -rf "$dest"
mkdir -p "$dest"
cp "$src"/imgui_touch*.h "$src"/imgui_touch*.cpp "$src"/LICENSE "$dest"/
sha=$(git -C "$src" rev-parse --short HEAD 2>/dev/null || echo "unknown")
echo "imgui_touch $sha (https://github.com/pthom/imgui_touch), vendored by tools/update_imgui_touch.sh" > "$dest/VERSION.txt"
echo "imgui_touch vendored from $src at $sha"
