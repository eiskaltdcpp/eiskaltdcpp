#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -e
set -x

if [ "${TARGET}" = "linux64" ]
then
    export CXXFLAGS="$(dpkg-buildflags --get CXXFLAGS) \
                     $(dpkg-buildflags --get CPPFLAGS)"
    export LDFLAGS="$(dpkg-buildflags --get LDFLAGS) -Wl,--as-needed"
    [ "${USE_QT}" = "qt5" ] && CXXFLAGS="${CXXFLAGS} -fPIC" || true

    CMAKE_OPTIONS=".. \
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

    du -shc /usr/bin/eiskaltdcpp-*
    du -shc /usr/lib*/libeiskaltdcpp.so*

    if [ -z "${USE_DAEMON}" ]
    then
        du -shc /usr/share/eiskaltdcpp/*
    fi
fi

if [ "${TARGET}" = "windows32" ] || [ "${TARGET}" = "windows64" ]
then
    CMAKE_OPTIONS=".. \
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
        -DUSE_CLI_JSONRPC=ON \
        "

    # Workaround for fixind of build with Boost from MXE packages
    sudo sed -i 's/std::sprintf/sprintf/' /usr/lib/mxe/usr/*-w64-mingw32.shared/include/boost/interprocess/detail/win32_api.hpp
    # End of workaround

    if [ "${TARGET}" = "windows64" ]
    then
        CMAKE_TOOL="/usr/lib/mxe/usr/bin/x86_64-w64-mingw32.shared-cmake"
    else
        CMAKE_TOOL="/usr/lib/mxe/usr/bin/i686-w64-mingw32.shared-cmake"
    fi

    mkdir -p builddir
    cd builddir

    ${CMAKE_TOOL} ${CMAKE_OPTIONS}
    make VERBOSE=1 -k -j $(nproc)
    make install -j 1

    du -shc ./EiskaltDC++/eiskaltdcpp-*
fi

if [ "${TARGET}" = "macos64" ]
then
    ls -alp /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
    ls -alp $(brew --repository)/*
    ls -alp /usr/local/*

    export HOMEBREW="/usr/local"
    ./macos/build-using-homebrew.sh

    du -shc ../EiskaltDC++*.dmg
fi

