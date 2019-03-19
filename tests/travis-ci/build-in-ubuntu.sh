#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -e
set -x

export CXXFLAGS="$(dpkg-buildflags --get CXXFLAGS) \
                 $(dpkg-buildflags --get CPPFLAGS)"
export LDFLAGS="$(dpkg-buildflags --get LDFLAGS) -Wl,--as-needed"
[ "${USE_QT}" = "qt5" ] && CXXFLAGS="${CXXFLAGS} -fPIC" || true

CMAKE_OPTIONS=".. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_MINIUPNP=ON \
    -DWITH_DHT=ON \
    -DUSE_IDNA=ON \
    -DLUA_SCRIPT=ON \
    -DWITH_LUASCRIPTS=ON \
    -DWITH_DEV_FILES=ON \
    -DPERL_REGEX=ON \
    -DWITH_SOUNDS=ON \
    "

if [ "${USE_QT}" = "qt4" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_QT=ON \
        -DUSE_QT5=OFF \
        "
elif [ "${USE_QT}" = "qt5" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DUSE_QT=OFF \
        -DUSE_QT5=ON \
        "
fi

if [ "${USE_QT}" = "qt4" ] || [ "${USE_QT}" = "qt5" ]
then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} \
        -DDBUS_NOTIFY=ON \
        -DWITH_EMOTICONS=ON \
        -DWITH_EXAMPLES=ON \
        -DUSE_JS=ON \
        -DUSE_QT_QML=ON \
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
        -DUSE_QT=OFF \
        -DUSE_QT5=OFF \
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
        -DUSE_QT=OFF \
        -DUSE_QT5=OFF \
        -DNO_UI_DAEMON=ON \
        -DXMLRPC_DAEMON=OFF \
        -DJSONRPC_DAEMON=ON \
        -DUSE_CLI_JSONRPC=ON \
        -DUSE_CLI_XMLRPC=OFF \
        "
fi

mkdir -p builddir
cd builddir

cmake ${CMAKE_OPTIONS} \
      -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
      -DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
      -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}"
make VERBOSE=1 -k -j $(nproc)
sudo make install -j 1

