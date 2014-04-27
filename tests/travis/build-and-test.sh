#!/bin/sh

set -x

export CXXFLAGS="-O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2"
export LDFLAGS="-Wl,-z,relro -Wl,--as-needed"

mkdir -p builddir
cd builddir

CMAKEOPTS="..
           -DCMAKE_INSTALL_PREFIX=/usr
           -DCMAKE_CXX_FLAGS=${CXXFLAGS}
           -DCMAKE_SHARED_LINKER_FLAGS=${LDFLAGS}
           -DCMAKE_EXE_LINKER_FLAGS=${LDFLAGS}
           -DCMAKE_BUILD_TYPE=RelWithDebInfo"

if [ "${CONFIG}" = "full" ]; then
    CMAKEOPTS="${CMAKEOPTS}
               -DUSE_MINIUPNP=ON
               -DLOCAL_MINIUPNP=OFF
               -DWITH_DHT=ON
               -DUSE_IDNA=ON
               -DLUA_SCRIPT=ON
               -DWITH_LUASCRIPTS=ON
               -DWITH_DEV_FILES=ON
               -DPERL_REGEX=ON
               -DWITH_SOUNDS=ON"
else
    CMAKEOPTS="${CMAKEOPTS}
               -Dlinguas=''
               -DUSE_MINIUPNP=OFF
               -DWITH_DHT=OFF
               -DUSE_IDNA=OFF
               -DLUA_SCRIPT=OFF
               -DWITH_LUASCRIPTS=OFF
               -DWITH_DEV_FILES=OFF
               -DPERL_REGEX=OFF
               -DWITH_SOUNDS=OFF"
fi


if [ ! -z "${USE_QT}" ]; then
    if [ "${USE_QT}" = "qt4" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DUSE_QT=ON
                   -DUSE_QT5=OFF"
    elif [ "${USE_QT}" = "qt5" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DUSE_QT=OFF
                   -DUSE_QT5=ON"
    fi
    if [ "${CONFIG}" = "full" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DDBUS_NOTIFY=ON
                   -DWITH_EMOTICONS=ON
                   -DWITH_EXAMPLES=ON
                   -DUSE_JS=ON
                   -DUSE_QT_QML=ON
                   -DUSE_ASPELL=ON
                   -DUSE_QT_SQLITE=ON"
    else
        CMAKEOPTS="${CMAKEOPTS}
                   -DDBUS_NOTIFY=OFF
                   -DWITH_EMOTICONS=OFF
                   -DWITH_EXAMPLES=OFF
                   -DUSE_JS=OFF
                   -DUSE_QT_QML=OFF
                   -DUSE_ASPELL=OFF
                   -DUSE_QT_SQLITE=OFF"
    fi
fi

if [ ! -z "${USE_GTK}" ]; then
    CMAKEOPTS="${CMAKEOPTS}
               -DUSE_QT=OFF
               -DUSE_LIBGNOME2=OFF
               -DCHECK_GTK_DEPRECATED=OFF"
    if [ "${USE_GTK}" = "gtk2" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DUSE_GTK=ON
                   -DUSE_GTK3=OFF"
    elif [ "${USE_GTK}" = "gtk3" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DUSE_GTK=OFF
                   -DUSE_GTK3=ON"
    fi
    if [ "${CONFIG}" = "full" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DUSE_LIBCANBERRA=ON
                   -DUSE_LIBNOTIFY=ON"
    else
        CMAKEOPTS="${CMAKEOPTS}
                   -DUSE_LIBCANBERRA=OFF
                   -DUSE_LIBNOTIFY=OFF"
    fi
fi

if [ ! -z "${USE_DAEMON}" ]; then
    CMAKEOPTS="${CMAKEOPTS}
               -DUSE_QT=OFF
               -DNO_UI_DAEMON=ON
               -DXMLRPC_DAEMON=OFF"
    if [ "${CONFIG}" = "full" ]; then
        CMAKEOPTS="${CMAKEOPTS}
                   -DJSONRPC_DAEMON=ON"
    fi
fi

if [ ! -z "${USE_CLI}" ]; then
    CMAKEOPTS="${CMAKEOPTS}
               -DUSE_QT=OFF
               -DUSE_CLI_JSONRPC=ON
               -DUSE_CLI_XMLRPC=OFF"
fi


cmake ${CMAKEOPTS}
make VERBOSE=1
sudo make install

du -shc /usr/bin/eiskaltdcpp-* /usr/lib*/libeiskaltdcpp.so* /usr/share/eiskaltdcpp/*

