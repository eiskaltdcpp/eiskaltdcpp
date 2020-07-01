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
    sudo add-apt-repository 'deb [arch=amd64] https://mirror.mxe.cc/repos/apt xenial main'
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 84C7C89FC632241A6999ED0A580873F586B72ED9

    if [ "${TARGET}" = "windows64" ]
    then
        PKG_PREFIX="mxe-x86-64-w64-mingw32.shared"
    else
        PKG_PREFIX="mxe-i686-w64-mingw32.shared"
    fi

    sudo apt-get update  -qq
    sudo apt-get install -qq ${PKG_PREFIX}-aspell \
                             ${PKG_PREFIX}-boost \
                             ${PKG_PREFIX}-jsoncpp \
                             ${PKG_PREFIX}-libidn \
                             ${PKG_PREFIX}-lua \
                             ${PKG_PREFIX}-miniupnpc \
                             ${PKG_PREFIX}-qtmultimedia \
                             ${PKG_PREFIX}-qttools
fi

if [ "${TARGET}" = "macos64" ]
then
    export HOMEBREW_NO_AUTO_UPDATE=1
    # brew install coreutils cmake gettext boost openssl pcre
    brew install ccache libidn jsoncpp miniupnpc aspell lua qt
fi

