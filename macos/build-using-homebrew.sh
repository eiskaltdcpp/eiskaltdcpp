#!/bin/sh

# Authors: Boris Pek
# License: Public Domain
# Created: 2018-08-21
# Created: 2018-08-23
# Version: N/A
#
# Description: script for personal use
#
# Notes:
# brew install wget git htop coreutils
# brew install cmake gettext boost libidn openssl jsoncpp miniupnpc aspell pcre lua qt

set -e

PATH="${HOMEBREW}/bin:${PATH}"
CUR_DIR="$(dirname $(realpath -s ${0}))"
MAIN_DIR="$(realpath -s ${CUR_DIR}/..)"
TOOLCHAIN_FILE="${CUR_DIR}/homebrew-toolchain.cmake"

BUILD_OPTIONS="-DCMAKE_BUILD_TYPE=Release \
               -DUSE_MINIUPNP=ON \
               -DLOCAL_MINIUPNP=OFF \
               -DFREE_SPACE_BAR_C=ON \
               -DWITH_SOUNDS=ON \
               -DWITH_LUASCRIPTS=ON \
               -DUSE_ASPELL=ON \
               -DPERL_REGEX=ON \
               -DLUA_SCRIPT=ON \
               -DWITH_DHT=ON \
               -DLOCAL_BOOST=OFF \
               -DUSE_QT=OFF \
               -DUSE_QT5=ON"

mkdir -p "${MAIN_DIR}/builddir"
cd "${MAIN_DIR}/builddir"

cmake .. -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" ${BUILD_OPTIONS} "$@"
cmake --build . --target all --parallel 4

cpack -G DragNDrop
cp -a EiskaltDC++*.dmg "${MAIN_DIR}/../"

echo
echo "App bundle is built successfully! See:"
echo "$(realpath -s ${MAIN_DIR}/..)/$(ls EiskaltDC++*.dmg | sort -V | tail -n1)"
echo
