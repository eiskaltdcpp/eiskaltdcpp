#!/bin/sh

# Authors: Maxim Ignatenko, Dmitry Arkhipov
# License: Public Domain
# Created: 2013-06-18
# Created: 2013-06-18
# Version: N/A
#
# Description: script for personal use

set -e

OSXSDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk"
OPTIONS="-DCMAKE_BUILD_TYPE=Release -DUSE_MINIUPNP=ON -DLOCAL_MINIUPNP=ON -DFREE_SPACE_BAR_C=ON -DWITH_SOUNDS=ON -DUSE_ASPELL=ON -DWITH_DHT=ON"
COMPILER="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
COMPILER_FLAGS="-DCMAKE_SYSTEM_PREFIX_PATH='/opt/local/;${OSXSDK}/' -DCMAKE_INCLUDE_PATH='${OSXSDK}/usr/include;/opt/local/include;/usr/include' -DCMAKE_CXX_FLAGS='-I${OSXSDK}/usr/include -I/opt/local/include -stdlib=libc++' -DCMAKE_C_FLAGS='-I${OSXSDK}/usr/include -I/opt/local/include' -DCMAKE_EXE_LINKER_FLAGS=-L/opt/local/lib"
ARCH="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.8 -DCMAKE_OSX_SYSROOT='${OSXSDK}'"

eval /opt/local/bin/cmake ../eiskaltdcpp -DCMAKE_OSX_ARCHITECTURES=\'x86_64\' ${ARCH} ${COMPILER_FLAGS} ${COMPILER} ${OPTIONS} "$@"

make -j 4

cpack -G DragNDrop

