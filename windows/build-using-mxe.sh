#!/bin/sh

# Authors: Boris Pek
# License: Public Domain
# Created: 2018-09-16
# Created: 2018-09-16
# Version: N/A
#
# Description: script for cross-compilation of program for MS Windows using MXE
# project. Currently it is used for testing builds on Travis CI.
#
# Notes:
# make aspell boost jsoncpp libidn lua miniupnpc qtbase qtmultimedia qttools

set -e

if [ -z "${MXE_DIR}" ]
then
    MXE_DIR="${HOME}/mxe"
fi

if [ -z "${MXE_TARGET}" ]
then
    MXE_TARGET="i686-w64-mingw32.shared"
fi

CUR_DIR="$(dirname $(realpath -s ${0}))"
MAIN_DIR="$(realpath -s ${CUR_DIR}/..)"

PATH="${MXE_DIR}/usr/bin:${PATH}"
CMAKE_TOOL="${MXE_TARGET}-cmake"

CMAKE_OPTIONS="-DCMAKE_INSTALL_PREFIX=../../EiskaltDC++ \
               -DCMAKE_BUILD_TYPE=Release \
               -DSHARE_DIR=resources \
               -DUSE_QT=OFF \
               -DUSE_QT5=ON \
               -DUSE_QT_SQLITE=ON \
               -DUSE_QT_QML=OFF \
               -DUSE_MINIUPNP=ON \
               -DUSE_ASPELL=ON \
               -DUSE_JS=OFF \
               -DUSE_CLI_JSONRPC=ON \
               -DFORCE_XDG=OFF \
               -DDBUS_NOTIFY=OFF \
               -DNO_UI_DAEMON=ON \
               -DJSONRPC_DAEMON=ON \
               -DPERL_REGEX=ON \
               -DLUA_SCRIPT=ON \
               -DWITH_SOUNDS=ON \
               -DWITH_LUASCRIPTS=ON \
               -DWITH_EXAMPLES=OFF \
               -DLOCAL_BOOST=OFF \
               -DLOCAL_ASPELL_DATA=OFF \
               -DLOCAL_JSONCPP=OFF"

mkdir -p "${MAIN_DIR}/builddir"
cd "${MAIN_DIR}/builddir"

${CMAKE_TOOL} .. ${CMAKE_OPTIONS} "$@"
make VERBOSE=1 -k -j $(nproc)
make install -j 1

echo
echo "Program is built successfully! See:"
echo "$(realpath -s ${MAIN_DIR}/..)/EiskaltDC++"
echo
