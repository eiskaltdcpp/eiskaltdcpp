#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -e
set -x

mkdir -p builddir
cd builddir

if [ "${TARGET_OS}" = "Windows" ]; then
    CMAKEOPTS=".. \
               -DCMAKE_INSTALL_PREFIX=./EiskaltDC++ \
               -DCMAKE_BUILD_TYPE=Release \
               -DSHARE_DIR=resources \
               -DDO_NOT_USE_MUTEX=ON \
               -DUSE_ASPELL=ON \
               -DFORCE_XDG=OFF \
               -DDBUS_NOTIFY=OFF \
               -DUSE_JS=OFF \
               -DWITH_EXAMPLES=OFF \
               -DUSE_MINIUPNP=ON \
               -DWITH_SOUNDS=ON \
               -DPERL_REGEX=ON \
               -DUSE_QT_QML=OFF \
               -DLUA_SCRIPT=ON \
               -DWITH_LUASCRIPTS=ON \
               -DUSE_QT_SQLITE=ON \
               -DNO_UI_DAEMON=ON \
               -DJSONRPC_DAEMON=ON \
               -DLOCAL_JSONCPP=OFF \
               -DUSE_CLI_JSONRPC=ON"

    # Workaround for fixind of build with Boost from MXE packages
    sudo sed -i 's/std::sprintf/sprintf/' /usr/lib/mxe/usr/x86_64-w64-mingw32.shared/include/boost/interprocess/detail/win32_api.hpp
    # End of workaround
    /usr/lib/mxe/usr/bin/x86_64-w64-mingw32.shared-cmake ${CMAKEOPTS}
    make VERBOSE=1 -k -j $(nproc)
    make install -j 1

    du -shc ./EiskaltDC++/eiskaltdcpp-*
else # Build for Ubuntu
    export CXXFLAGS="$(dpkg-buildflags --get CXXFLAGS) $(dpkg-buildflags --get CPPFLAGS)"
    export LDFLAGS="$(dpkg-buildflags --get LDFLAGS) -Wl,--as-needed"
    if [ "${USE_QT}" = "qt5" ]; then
        CXXFLAGS="${CXXFLAGS} -fPIC"
    fi

    CMAKEOPTS=".. \
               -DCMAKE_INSTALL_PREFIX=/usr \
               -DCMAKE_BUILD_TYPE=Release \
               -DUSE_MINIUPNP=ON \
               -DLOCAL_MINIUPNP=OFF \
               -DWITH_DHT=ON \
               -DUSE_IDNA=ON \
               -DLUA_SCRIPT=ON \
               -DWITH_LUASCRIPTS=ON \
               -DWITH_DEV_FILES=ON \
               -DPERL_REGEX=ON \
               -DWITH_SOUNDS=ON"

    if [ ! -z "${USE_QT}" ]; then
        if [ "${USE_QT}" = "qt4" ]; then
            CMAKEOPTS="${CMAKEOPTS} \
                       -DUSE_QT=ON \
                       -DUSE_QT5=OFF"
        elif [ "${USE_QT}" = "qt5" ]; then
            CMAKEOPTS="${CMAKEOPTS} \
                       -DUSE_QT=OFF \
                       -DUSE_QT5=ON"
        fi
            CMAKEOPTS="${CMAKEOPTS} \
                       -DDBUS_NOTIFY=ON \
                       -DWITH_EMOTICONS=ON \
                       -DWITH_EXAMPLES=ON \
                       -DUSE_JS=ON \
                       -DUSE_QT_QML=ON \
                       -DUSE_ASPELL=ON \
                       -DUSE_QT_SQLITE=ON"
    fi

    if [ ! -z "${USE_GTK}" ]; then
        CMAKEOPTS="${CMAKEOPTS} \
                   -DUSE_QT=OFF \
                   -DUSE_QT5=OFF \
                   -DUSE_LIBGNOME2=OFF \
                   -DCHECK_GTK_DEPRECATED=OFF"
        if [ "${USE_GTK}" = "gtk2" ]; then
            CMAKEOPTS="${CMAKEOPTS} \
                       -DUSE_GTK=ON \
                       -DUSE_GTK3=OFF"
        elif [ "${USE_GTK}" = "gtk3" ]; then
            CMAKEOPTS="${CMAKEOPTS} \
                       -DUSE_GTK=OFF \
                       -DUSE_GTK3=ON"
        fi
            CMAKEOPTS="${CMAKEOPTS} \
                       -DUSE_LIBCANBERRA=ON \
                       -DUSE_LIBNOTIFY=ON \
                       -DUSE_ASPELL=OFF"
    fi

    if [ ! -z "${USE_DAEMON}" ]; then
        CMAKEOPTS="${CMAKEOPTS} \
                   -DUSE_QT=OFF \
                   -DUSE_QT5=OFF \
                   -DNO_UI_DAEMON=ON \
                   -DXMLRPC_DAEMON=OFF \
                   -DJSONRPC_DAEMON=ON \
                   -DUSE_CLI_JSONRPC=ON \
                   -DUSE_CLI_XMLRPC=OFF"
    fi

    cmake ${CMAKEOPTS} \
          -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
          -DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
          -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}"
    make VERBOSE=1 -k -j $(nproc)
    sudo make install -j 1

    du -shc /usr/bin/eiskaltdcpp-*
    du -shc /usr/lib*/libeiskaltdcpp.so*

    if [ -z "${USE_DAEMON}" ]; then
        du -shc /usr/share/eiskaltdcpp/*
    fi
fi
