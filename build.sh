#!/bin/sh
# FastCalc build script.
#
# Usage:
#   ./build.sh                 configure + build (Release) + run tests
#   ./build.sh --clean         wipe build dirs before building
#   ./build.sh --sanitize      build with ASan+UBSan into build-san/
#   ./build.sh --debug         Debug build instead of Release
#   ./build.sh -j N            parallel build jobs (default: nproc)
#   ./build.sh -h | --help     show help
#
# Output binary: build/fastcalc  (or build-san/fastcalc with --sanitize)

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_TYPE="Release"
SANITIZE=0
CLEAN=0
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=4
fi

usage() {
    sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

while [ $# -gt 0 ]; do
    case "$1" in
        --clean) CLEAN=1 ;;
        --sanitize) SANITIZE=1 ;;
        --debug) BUILD_TYPE="Debug" ;;
        -j|--jobs)
            JOBS="$2"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "build.sh: unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

need() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "build.sh: missing required tool: $1" >&2
        exit 1
    fi
}

need cmake
need pkg-config

for dep in sdl3 freetype2; do
    if ! pkg-config --exists "$dep"; then
        echo "build.sh: missing dependency: $dep" >&2
        echo "  Ubuntu/Debian: sudo apt install libsdl3-dev libfreetype-dev" >&2
        exit 1
    fi
done

if [ "$SANITIZE" -eq 1 ]; then
    BUILD_DIR="$ROOT/build-san"
    SAN_FLAG="ON"
else
    BUILD_DIR="$ROOT/build"
    SAN_FLAG="OFF"
fi

if [ "$CLEAN" -eq 1 ]; then
    echo "==> removing $ROOT/build $ROOT/build-san"
    rm -rf "$ROOT/build" "$ROOT/build-san"
fi

echo "==> configuring ($BUILD_TYPE, SANITIZE=$SAN_FLAG) in $BUILD_DIR"
cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSANITIZE="$SAN_FLAG"

echo "==> building (-j$JOBS)"
cmake --build "$BUILD_DIR" -j "$JOBS"

echo "==> testing"
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "==> done: $BUILD_DIR/fastcalc"
