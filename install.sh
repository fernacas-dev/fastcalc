#!/bin/sh
# FastCalc installer for Ubuntu.
#
# Usage:
#   ./install.sh                    build (Release) + install to /usr/local
#   ./install.sh --prefix DIR       install to DIR instead of /usr/local
#   ./install.sh --no-build         skip the build, install existing binaries
#   ./install.sh --deps             install build dependencies via apt first
#   ./install.sh --uninstall        remove installed files
#   ./install.sh -h | --help        show help
#
# Installs:
#   <prefix>/bin/fastcalc
#   <prefix>/share/applications/fastcalc.desktop
#   <prefix>/share/icons/hicolor/scalable/apps/fastcalc.svg
#
# Uses sudo automatically when the prefix is not writable.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PREFIX="/usr/local"
UNINSTALL=0
NO_BUILD=0
INSTALL_DEPS=0

usage() {
    sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix)
            PREFIX="$2"
            shift
            ;;
        --uninstall) UNINSTALL=1 ;;
        --no-build) NO_BUILD=1 ;;
        --deps) INSTALL_DEPS=1 ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "install.sh: unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

BIN_FILE="$PREFIX/bin/fastcalc"
DESKTOP_FILE="$PREFIX/share/applications/fastcalc.desktop"
ICON_FILE="$PREFIX/share/icons/hicolor/scalable/apps/fastcalc.svg"

# sudo only when needed. A missing prefix is creatable when its nearest
# existing parent is writable.
probe="$PREFIX"
while [ ! -e "$probe" ]; do
    probe=$(dirname "$probe")
done
SUDO=""
if [ ! -w "$probe" ] && [ "$(id -u)" -ne 0 ]; then
    if ! command -v sudo >/dev/null 2>&1; then
        echo "install.sh: $PREFIX is not writable and sudo is unavailable" >&2
        echo "  Use --prefix with a writable directory (e.g. --prefix ~/.local)." >&2
        exit 1
    fi
    SUDO="sudo"
fi

refresh_caches() {
    $SUDO update-desktop-database "$PREFIX/share/applications" >/dev/null 2>&1 || true
    $SUDO gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" >/dev/null 2>&1 || true
}

if [ "$UNINSTALL" -eq 1 ]; then
    echo "==> removing FastCalc from $PREFIX"
    $SUDO rm -f "$BIN_FILE" "$DESKTOP_FILE" "$ICON_FILE"
    refresh_caches
    echo "==> uninstalled"
    exit 0
fi

if [ "$INSTALL_DEPS" -eq 1 ]; then
    echo "==> installing build dependencies via apt"
    sudo apt-get update
    sudo apt-get install -y build-essential cmake pkg-config \
        libsdl3-dev libfreetype-dev desktop-file-utils
fi

if [ "$NO_BUILD" -eq 0 ]; then
    echo "==> building (Release)"
    "$ROOT/build.sh"
else
    if [ ! -x "$ROOT/build/fastcalc" ]; then
        echo "install.sh: no binary at $ROOT/build/fastcalc (run without --no-build first)" >&2
        exit 1
    fi
fi

echo "==> installing to $PREFIX"
$SUDO cmake --install "$ROOT/build" --prefix "$PREFIX"
refresh_caches

echo "==> installed:"
echo "    $BIN_FILE"
echo "    $DESKTOP_FILE"
echo "    $ICON_FILE"
echo "Run with: fastcalc"
