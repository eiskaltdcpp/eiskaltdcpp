#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -e
set -x

export CXXFLAGS="$(dpkg-buildflags --get CXXFLAGS) \
                 $(dpkg-buildflags --get CPPFLAGS)"
export LDFLAGS="$(dpkg-buildflags --get LDFLAGS) -Wl,--as-needed"
[ "${USE_QT}" = "qt6" ] && CXXFLAGS="${CXXFLAGS} -fPIC" || true

CMAKE_OPTIONS=".. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_MINIUPNP=ON \
    -DWITH_DHT=ON \
    -DUSE_IDN2=ON \
    -DUSE_XATTR=ON \
    -DLUA_SCRIPT=ON \
    -DWITH_LUASCRIPTS=ON \
    -DWITH_DEV_FILES=ON \
    -DPERL_REGEX=ON \
    -DWITH_SOUNDS=ON \
    "

if [ "${USE_QT}" = "qt6" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_QT6=ON \
        -DUSE_QT_QML=OFF \
        "
fi

if [ "${USE_QT}" = "qt6" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DDBUS_NOTIFY=ON \
        -DWITH_EMOTICONS=ON \
        -DWITH_EXAMPLES=ON \
        -DUSE_JS=ON \
        -DUSE_ASPELL=ON \
        -DUSE_QT_SQLITE=ON \
        "
fi

if [ "${USE_GTK}" = "gtk2" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_GTK=ON \
        -DUSE_GTK3=OFF \
        "
elif [ "${USE_GTK}" = "gtk3" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_GTK=OFF \
        -DUSE_GTK3=ON \
        "
fi

if [ "${USE_GTK}" = "gtk2" ] || [ "${USE_GTK}" = "gtk3" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_QT6=OFF \
        -DUSE_LIBGNOME2=OFF \
        -DCHECK_GTK_DEPRECATED=OFF \
        -DUSE_LIBCANBERRA=ON \
        -DUSE_LIBNOTIFY=ON \
        -DUSE_ASPELL=OFF \
        "
fi

if [ "${USE_DAEMON}" = "jsonrpc" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_QT6=OFF \
        -DNO_UI_DAEMON=ON \
        -DXMLRPC_DAEMON=OFF \
        -DJSONRPC_DAEMON=ON \
        -DUSE_CLI_JSONRPC=ON \
        -DUSE_CLI_XMLRPC=OFF \
        -DLOCAL_JSONCPP=OFF \
        "
fi

mkdir -p builddir
cd builddir

cmake ${CMAKE_OPTIONS} \
      -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
      -DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
      -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}"
make VERBOSE=1 -k -j $(nproc)
# sudo make install -j 1

