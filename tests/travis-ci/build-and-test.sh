#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -e
set -x

if [ "${TARGET}" = "linux64" ]
then
    ./linux/build-in-ubuntu.sh

    ls -alp /usr/bin/eiskaltdcpp-*
    ls -alp /usr/lib*/libeiskaltdcpp.so*
    ls -alp /usr/share/eiskaltdcpp/*

    du -shc /usr/bin/eiskaltdcpp-*
    du -shc /usr/lib*/libeiskaltdcpp.so*
    du -shc /usr/share/eiskaltdcpp/*
fi

if [ "${TARGET}" = "windows32" ] || [ "${TARGET}" = "windows64" ]
then
    ls -alp /usr/lib/mxe/*
    ls -alp /usr/lib/mxe/usr/bin/*
    ls -alp /usr/lib/mxe/usr/*/bin/*

    # Workaround for fixind of build with Boost from MXE packages
    # See: https://github.com/mxe/mxe/issues/1104
    sudo sed -i 's/std::sprintf/sprintf/' /usr/lib/mxe/usr/*-w64-mingw32.shared/include/boost/interprocess/detail/win32_api.hpp
    # End of workaround

    if [ "${TARGET}" = "windows64" ]
    then
        export MXE_TARGET="x86_64-w64-mingw32.shared"
    else
        export MXE_TARGET="i686-w64-mingw32.shared"
    fi

    export MXE_DIR="/usr/lib/mxe"
    ./windows/build-using-mxe.sh

    ls -alp ../EiskaltDC++
    du -shc ../EiskaltDC++
fi

if [ "${TARGET}" = "macos64" ]
then
    ls -alp /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
    ls -alp $(brew --repository)/*
    ls -alp /usr/local/*

    export HOMEBREW="/usr/local"
    ./macos/build-using-homebrew.sh

    ls -alp ../EiskaltDC++*.dmg
    du -shc ../EiskaltDC++*.dmg
fi

