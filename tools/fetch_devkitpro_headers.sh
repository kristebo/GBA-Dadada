#!/usr/bin/env bash
# Copies the libgba headers/libs out of the pinned devkitARM Podman image
# onto the host, so CLion (via CMakeLists.txt) can resolve <gba.h> and
# friends for source navigation. The actual ROM build never uses this
# directory; it always runs inside the container through ./build.sh.
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="localhost/tg2027-devkitarm:20260610"
DEST="${1:-$ROOT/.devkitpro}"

if ! command -v podman >/dev/null 2>&1; then
    printf 'Error: Podman is required to fetch the headers.\n' >&2
    exit 1
fi

podman build --pull=missing --tag "$IMAGE" --file "$ROOT/Dockerfile" "$ROOT"

mkdir -p "$DEST"
rm -rf "$DEST/libgba"
podman run --rm --userns=keep-id \
    --volume "$DEST:/dest:Z" \
    "$IMAGE" cp -r /opt/devkitpro/libgba /dest/libgba

printf 'Fetched libgba headers/libs to %s/libgba\n' "$DEST"
printf 'Point CLion at them by exporting DEVKITPRO=%s before launching it,\n' "$DEST"
printf 'or reload the CMake project after setting that environment variable.\n'
