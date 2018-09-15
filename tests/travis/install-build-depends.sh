#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -e
set -x

if [ "${TARGET}" = "linux64" ]
then
    sudo apt-get update  -qq
    sudo apt-get install -qq cmake \
                             libbz2-dev \
                             libboost-dev \
                             libboost-system-dev \
                             libssl-dev \
                             libattr1-dev \
                             zlib1g-dev \
                             libidn11-dev \
                             liblua5.1-0-dev \
                             libpcre3-dev \
                             libminiupnpc-dev

    if [ "${USE_QT}" = "qt4" ]
    then
        sudo apt-get install -qq libqt4-dev \
                                 qt4-dev-tools \
                                 libaspell-dev
    elif [ "${USE_QT}" = "qt5" ]
    then
        sudo apt-get install -qq qtbase5-dev \
                                 qttools5-dev \
                                 qtmultimedia5-dev \
                                 qtquick1-5-dev \
                                 qtscript5-dev \
                                 qt5-default \
                                 libqt5xmlpatterns5-dev \
                                 qttools5-dev-tools \
                                 libaspell-dev
    fi

    if [ "${USE_GTK}" = "gtk2" ]
    then
        sudo apt-get install -qq libgtk2.0-dev \
                                 libnotify-dev \
                                 libcanberra-gtk-dev
    elif [ "${USE_GTK}" = "gtk3" ]
    then
        sudo apt-get install -qq libgtk-3-dev \
                                 libnotify-dev \
                                 libcanberra-gtk3-dev
    fi

    if [ "${USE_DAEMON}" = "jsonrpc" ]
    then
        sudo apt-get install -qq libdata-dump-perl \
                                 libgetopt-long-descriptive-perl \
                                 libjson-rpc-perl \
                                 libterm-shellui-perl
    fi
fi

if [ "${TARGET}" = "windows32" ] || [ "${TARGET}" = "windows64" ]
then
    # Add debian packages built from MXE packages
    echo "deb http://pkg.mxe.cc/repos/apt/debian wheezy main" | sudo tee --append /etc/apt/sources.list.d/mxeapt.list
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys D43A795B73B16ABE9643FE1AFD8FFF16DB45C6AB
fi

if [ "${TARGET}" = "windows32" ]
then
    sudo apt-get update  -qq
    sudo apt-get install -qq mxe-i686-w64-mingw32.shared-aspell \
                             mxe-i686-w64-mingw32.shared-boost \
                             mxe-i686-w64-mingw32.shared-jsoncpp \
                             mxe-i686-w64-mingw32.shared-libidn \
                             mxe-i686-w64-mingw32.shared-lua \
                             mxe-i686-w64-mingw32.shared-miniupnpc \
                             mxe-i686-w64-mingw32.shared-qtmultimedia \
                             mxe-i686-w64-mingw32.shared-qttools
fi

if [ "${TARGET}" = "windows64" ]
then
    sudo apt-get update  -qq
    sudo apt-get install -qq mxe-x86-64-w64-mingw32.shared-aspell \
                             mxe-x86-64-w64-mingw32.shared-boost \
                             mxe-x86-64-w64-mingw32.shared-jsoncpp \
                             mxe-x86-64-w64-mingw32.shared-libidn \
                             mxe-x86-64-w64-mingw32.shared-lua \
                             mxe-x86-64-w64-mingw32.shared-miniupnpc \
                             mxe-x86-64-w64-mingw32.shared-qtmultimedia \
                             mxe-x86-64-w64-mingw32.shared-qttools
fi

if [ "${TARGET}" = "macos64" ]
then
    # brew install coreutils cmake gettext boost openssl pcre
    brew install libidn jsoncpp miniupnpc aspell lua qt || true
fi

