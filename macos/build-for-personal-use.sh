#!/bin/sh

# Authors: Maxim Ignatenko, Dmitry Arkhipov
# License: Public Domain
# Created: 2013-06-18
# Updated: 2013-06-18
# Version: N/A
#
# Description: script for personal use

set -e

OSXSDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk"
OPTIONS="-DCMAKE_BUILD_TYPE=Release -DUSE_MINIUPNP=ON -DFREE_SPACE_BAR_C=ON -DWITH_SOUNDS=ON -DUSE_ASPELL=ON -DWITH_DHT=ON -DUSE_QT=OFF -DUSE_QT5=ON"
COMPILER="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
COMPILER_FLAGS="-DCMAKE_SYSTEM_PREFIX_PATH='/usr/local/;${OSXSDK}/' -DCMAKE_INCLUDE_PATH='${OSXSDK}/usr/include;/usr/local/include;/usr/include' -DCMAKE_CXX_FLAGS='-I${OSXSDK}/usr/include -I/usr/local/include -stdlib=libc++' -DCMAKE_C_FLAGS='-I${OSXSDK}/usr/include -I/usr/local/include' -DCMAKE_EXE_LINKER_FLAGS=-L/usr/local/lib"
ARCH="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 -DCMAKE_OSX_SYSROOT='${OSXSDK}'"
GETTEXT="-DGETTEXT_SEARCH_PATH=/usr/local/opt/gettext/bin"
OPENSSL="-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include"

eval cmake .. -DCMAKE_OSX_ARCHITECTURES=\'x86_64\' ${ARCH} ${COMPILER_FLAGS} ${COMPILER} ${OPTIONS} ${GETTEXT} ${OPENSSL} "$@"

make -j 4

cpack -G DragNDrop

