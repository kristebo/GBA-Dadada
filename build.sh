#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
IMAGE="localhost/tg2027-devkitarm:20260610"

if ! command -v podman >/dev/null 2>&1; then
    printf 'Error: Podman is required to build the ROM.\n' >&2
    exit 1
fi

podman build --pull=missing --tag "$IMAGE" --file "$ROOT/Dockerfile" "$ROOT"
podman run --rm --userns=keep-id \
    --volume "$ROOT:/work:Z" \
    --workdir /work \
    "$IMAGE" make "$@"

if [[ $# -eq 0 ]]; then
    test -s "$ROOT/rom/tg2027.gba"
    printf 'Built %s/rom/tg2027.gba\n' "$ROOT"
fi
