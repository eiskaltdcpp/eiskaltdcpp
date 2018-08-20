#!/bin/sh

# Author:  Maxim Ignatenko
# License: Public Domain
# Created: 2013-06-18
# Created: 2013-06-18
# Version: N/A
#
# Description: script for building universal application bundle

set -e

export PATH=/opt/local/bin:/opt/local/sbin:$PATH

OSXSDK=/Developer/SDKs/MacOSX10.7.sdk
OPTIONS="-DCMAKE_BUILD_TYPE=Release -DUSE_MINIUPNP=ON -DLOCAL_MINIUPNP=ON -DFREE_SPACE_BAR_C=ON -DWITH_SOUNDS=ON -DUSE_ASPELL=ON -DPERL_REGEX=ON -DWITH_DHT=ON -DLOCAL_BOOST=ON "
COMPILER="-DCMAKE_C_COMPILER=/opt/local/bin/gcc-mp -DCMAKE_CXX_COMPILER=/opt/local/bin/g++-mp"
COMPILER_FLAGS="-DCMAKE_SYSTEM_PREFIX_PATH='/opt/local/;${OSXSDK}/' -DCMAKE_INCLUDE_PATH='${OSXSDK}/usr/include;/opt/local/include;/usr/include' -DCMAKE_CXX_FLAGS='-I${OSXSDK}/usr/include -I/opt/local/include' -DCMAKE_C_FLAGS='-I${OSXSDK}/usr/include -I/opt/local/include' -DCMAKE_EXE_LINKER_FLAGS=-L/opt/local/lib"
ARCH="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.7 -DCMAKE_OSX_SYSROOT='${OSXSDK}'"

eval /opt/local/bin/cmake .. -DCMAKE_OSX_ARCHITECTURES=\'i386\;x86_64\' ${ARCH} ${COMPILER_FLAGS} ${COMPILER} ${OPTIONS} "$@"

make -j 4

cpack -G DragNDrop
