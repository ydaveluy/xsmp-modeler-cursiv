#!/usr/bin/env bash
# Packages the native server of a build as the VS Code extension of one
# platform, and the TAS-MDK handler as the TAS-MDK extension of that platform.
# A native server on Windows loads no handler, so Windows gets no TAS-MDK
# package. Run it from a clean checkout: it replaces vscode/bin, vscode/server
# and extensions/tas-mdk/server.
#
#   tools/package-native.sh <vsce target> <build directory>
#
# Linux and Alpine servers carry the libc++ they were built with, found
# through `$ORIGIN`, so that they run on systems without it and the handler
# shares it.
set -euo pipefail

target=$1
build=$(cd "$2" && pwd)
root=$(cd "$(dirname "$0")/.." && pwd)
vscode=$root/vscode
tas=$root/extensions/tas-mdk

notice() { cp "$1" "$vscode/bin/$2"; }
source_dir() {
  sed -n "s/^$1:[A-Z]*=//p" "$build/CMakeCache.txt" | head -n 1
}

rm -rf "$vscode/bin" "$vscode/server"
mkdir -p "$vscode/bin"
server=xsmp_lsp
[[ $target == win32-* ]] && server=xsmp_lsp.exe
cp "$build/src/$server" "$vscode/bin/"

# The notices of what the server links in. The WASI libc is not part of it.
cursiv=$(source_dir CURSIV_SOURCE_DIR)
[[ -n $cursiv ]] || cursiv=$build/_deps/cursiv-src
notice "$cursiv/LICENSE" cursiv.LICENSE
notice "$build/_deps/lsp_framework-src/LICENSE" lsp-framework.LICENSE
notice "$cursiv/cmake/patches/mimalloc.LICENSE" mimalloc.LICENSE
rm -f "$vscode"/licenses/cloudlibc-* "$vscode"/licenses/musl-*

case $target in
  linux-* | alpine-*)
    ldd "$vscode/bin/$server" | awk '/libc\+\+|libunwind/ { print $3 }' |
      while read -r library; do
        cp -L "$library" "$vscode/bin/"
        chmod u+w "$vscode/bin/$(basename "$library")"
        patchelf --set-rpath '$ORIGIN' "$vscode/bin/$(basename "$library")"
      done
    patchelf --set-rpath '$ORIGIN' "$vscode/bin/$server"
    notice "$root/tools/licenses/libcxx-LICENSE.txt" libc++.LICENSE
    ;;
  win32-*)
    # Windows: a static executable, with the C++ runtime under the GCC
    # Runtime Library Exception and the MinGW-w64 threads under their notice.
    license=$(find "$MINGW_PREFIX/share/licenses" -ipath '*winpthread*' -type f \
      -print -quit)
    [[ -n $license ]] || { echo "No winpthreads notice in $MINGW_PREFIX" >&2; exit 1; }
    notice "$license" winpthreads.LICENSE
    ;;
esac
(cd "$vscode" && npm run package -- --target "$target")

[[ $target == win32-* ]] && exit 0
rm -rf "$tas/server"
mkdir -p "$tas/server"
rm -f "$tas"/licenses/cloudlibc-* "$tas"/licenses/musl-*
cp "$build/extensions/tas-mdk/xsmp-tas-mdk-handler.so" "$tas/server/tas-mdk.so"
(cd "$tas" && npm run package -- --target "$target")
