#!/bin/sh

# Authors: Boris Pek
# License: Public Domain
# Created: 2018-08-21
# Updated: 2018-12-19
# Version: N/A
#
# Description: script for building of app bundles for macOS
# Currently it is used for testing builds on Travis CI and for producing
# official builds of program which are hosted on SourceForge.
#
# Notes:
# brew install --build-bottle pkg-config htop cmake coreutils gettext
# brew install --build-bottle aspell boost lua miniupnpc openssl pcre pcre2
# brew install --build-bottle libunistring libidn libidn2 jsoncpp qt
# brew install --build-bottle curl wget git

set -e

PATH="${HOMEBREW}/bin:${PATH}"
CUR_DIR="$(dirname $(realpath -s ${0}))"
MAIN_DIR="$(realpath -s ${CUR_DIR}/..)"
TOOLCHAIN_FILE="${CUR_DIR}/homebrew-toolchain.cmake"

BUILD_OPTIONS="-DCMAKE_BUILD_TYPE=Release \
               -DUSE_QT=OFF \
               -DUSE_QT5=ON \
               -DUSE_QT_SQLITE=ON \
               -DUSE_MINIUPNP=ON \
               -DUSE_ASPELL=ON \
               -DNO_UI_DAEMON=OFF \
               -DJSONRPC_DAEMON=OFF \
               -DPERL_REGEX=ON \
               -DLUA_SCRIPT=ON \
               -DWITH_SOUNDS=ON \
               -DWITH_LUASCRIPTS=ON \
               -DLOCAL_MINIUPNP=OFF \
               -DLOCAL_BOOST=OFF \
               -DLOCAL_ASPELL_DATA=OFF"

mkdir -p "${MAIN_DIR}/builddir"
cd "${MAIN_DIR}/builddir"

cmake .. -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" ${BUILD_OPTIONS} "$@"
cmake --build . --target all -- -j4

cpack -G DragNDrop
cp -a EiskaltDC++*.dmg "${MAIN_DIR}/../"

echo
echo "App bundle is built successfully! See:"
echo "$(realpath -s ${MAIN_DIR}/..)/$(ls EiskaltDC++*.dmg | sort -V | tail -n1)"
echo
