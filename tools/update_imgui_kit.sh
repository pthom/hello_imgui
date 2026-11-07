#!/usr/bin/env bash
# Refreshes the vendored copy of imgui_kit (external/imgui_kit/imgui_kit) from a checkout of
# https://github.com/pthom/imgui_kit: its header and sources only (no tests, no CI).
# Usage: tools/update_imgui_kit.sh <path to an imgui_kit checkout>
set -euo pipefail
src=${1:?"usage: $0 <path to an imgui_kit checkout>"}
here=$(cd "$(dirname "$0")/.." && pwd)
dest="$here/external/imgui_kit/imgui_kit"
rm -rf "$dest"
mkdir -p "$dest"
cp "$src"/imgui_kit*.h "$src"/imgui_kit*.cpp "$src"/LICENSE "$dest"/
sha=$(git -C "$src" rev-parse --short HEAD 2>/dev/null || echo "unknown")
echo "imgui_kit $sha (https://github.com/pthom/imgui_kit), vendored by tools/update_imgui_kit.sh" > "$dest/VERSION.txt"
echo "imgui_kit vendored from $src at $sha"
