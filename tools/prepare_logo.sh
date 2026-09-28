#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE="$ROOT/assets/pixel-art-2026-09-28T14-29-57.png"
OUTPUT="$ROOT/assets/graphics/tg_logo.png"

if ! command -v magick >/dev/null 2>&1; then
    printf 'Error: ImageMagick (magick) is required to prepare the logo.\n' >&2
    exit 1
fi

magick "$SOURCE" \
    -fuzz 2% -transparent white \
    -crop 140x92+16+40 +repage \
    -filter point -resize 64x42! \
    -background none -gravity center -extent 64x64 \
    -colors 16 "PNG32:$OUTPUT"
