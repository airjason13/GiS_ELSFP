#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=unknown
if git -C "$root" rev-parse --verify HEAD >/dev/null 2>&1; then
    version=$(git -C "$root" rev-parse --short=12 HEAD)
fi
build_time=$(date '+%Y-%m-%d %H:%M:%S %z')
output="$root/App/Common/build_version.h"
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT HUP INT TERM
printf '#ifndef BUILD_VERSION_H\n#define BUILD_VERSION_H\n#define FIRMWARE_BUILD_ID "%s"\n#define FIRMWARE_BUILD_TIME "%s"\n#endif\n' "$version" "$build_time" > "$tmp"
if ! cmp -s "$tmp" "$output"; then
    cp "$tmp" "$output"
fi
