#!/bin/sh

# Author:  Boris Pek
# Version: N/A
# License: Public Domain

set -x

# Add debian packages built from MXE packages
if [ "${OS}" = "mingw" ]; then
    echo "deb http://pkg.mxe.cc/repos/apt/debian wheezy main" | sudo tee --append /etc/apt/sources.list.d/mxeapt.list
    sudo apt-key adv --keyserver x-hkp://keys.gnupg.net --recv-keys D43A795B73B16ABE9643FE1AFD8FFF16DB45C6AB
fi

# This  is an ugly hack for partial updating of build environment from
# Ubuntu 12.04 (Precise Pangolin) to Ubuntu 14.04 (Trusty Tahr):
if [ "${OS}" != "mingw" ]; then
    sudo sed -i 's/precise/trusty/g' /etc/apt/sources.list
fi

sudo apt-get update -qq

# Common build dependencies:
if [ "${OS}" != "mingw" ]; then
    sudo apt-get install -qq cmake \
                             libbz2-dev \
                             libboost-dev \
                             libboost-system-dev \
                             libssl-dev \
                             libattr1-dev \
                             zlib1g-dev
fi

if [ "${CONFIG}" = "full" ]; then
    sudo apt-get install -qq libidn11-dev \
                             liblua5.1-0-dev \
                             libpcre3-dev \
                             libminiupnpc-dev
fi

if [ "${USE_QT}" = "qt4" ]; then
    sudo apt-get install -qq libqt4-dev \
                             qt4-dev-tools
    if [ "${CONFIG}" = "full" ]; then
        sudo apt-get install -qq libaspell-dev
    fi
elif [ "${USE_QT}" = "qt5" ]; then
    yes | sudo apt-get --yes --force-yes install -qq qtbase5-dev \
                             qttools5-dev \
                             qtmultimedia5-dev \
                             qtquick1-5-dev \
                             qtscript5-dev \
                             qt5-default \
                             libqt5xmlpatterns5-dev \
                             qttools5-dev-tools
    if [ "${CONFIG}" = "full" ]; then
        sudo apt-get install -qq libaspell-dev
    fi
fi

if [ "${USE_GTK}" = "gtk2" ]; then
    sudo apt-get install -qq libgtk2.0-dev
    if [ "${CONFIG}" = "full" ]; then
        sudo apt-get install -qq libnotify-dev \
                                 libcanberra-gtk-dev
    fi
elif [ "${USE_GTK}" = "gtk3" ]; then
    yes | sudo apt-get --yes --force-yes install -qq libgtk-3-dev
    if [ "${CONFIG}" = "full" ]; then
        sudo apt-get install -qq libnotify-dev \
                                 libcanberra-gtk3-dev
    fi
fi

if [ "${USE_DAEMON}" = "ON" ]; then
    echo "There are no additional build dependencies for daemon yet."
fi

if [ "${USE_CLI}" = "ON" ]; then
    sudo apt-get install -qq libdata-dump-perl \
                             libgetopt-long-descriptive-perl \
                             libjson-rpc-perl \
                             libterm-shellui-perl
fi

if [ "${OS}" = "mingw" ]; then
    sudo apt-get install -qq mxe-x86-64-w64-mingw32.shared-aspell \
                             mxe-x86-64-w64-mingw32.shared-boost \
                             mxe-x86-64-w64-mingw32.shared-jsoncpp \
                             mxe-x86-64-w64-mingw32.shared-libidn \
                             mxe-x86-64-w64-mingw32.shared-lua \
                             mxe-x86-64-w64-mingw32.shared-miniupnpc \
                             mxe-x86-64-w64-mingw32.shared-qtmultimedia \
                             mxe-x86-64-w64-mingw32.shared-qttools \
fi
