#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2021-01-24
# Version: N/A
#
# Dependencies:
# git, rsync, find, sed, appimagetool
# MXE: https://github.com/sibuserv/lxe/tree/hobby
# Sibuserv: https://github.com/sibuserv/sibuserv
#
# Current versions of used libraries may be found here:
# https://github.com/sibuserv/lxe/blob/hobby/etc/Ubuntu-14.04_amd64_static.sh
# https://github.com/sibuserv/lxe/blob/hobby/etc/Ubuntu-14.04_i386_static.sh

set -e

export MAIN_DIR="${HOME}/Tmp/EiskaltDC++"

CUR_DIR="$(dirname $(realpath -s ${0}))"
. "${CUR_DIR}/downloads-library.sh"
. "${CUR_DIR}/common-functions.sh"

BUILD_TARGETS="Ubuntu-14.04_amd64_static Ubuntu-14.04_i386_static"

# Script body

SCRIPT_NAME="$(basename ${0})"
ShowHelp ${@}

TestInternetConnection
PrepareMainDir

echo "Getting the sources..."
echo;

GetProgramSources ${@}
GetProgramVersion ${@}

echo "Preparing to build..."
PrepareToBuildForLinux
CleanBuildDir
echo "Done."
echo;

echo "Building EiskaltDC++..."
BuildProjectUsingSibuserv
echo;

echo "Installing..."
InstallToTempDir
echo;

echo "Preparing application directories..."
PrepareAppDirs
echo "Done."
echo;

echo "Building AppImage files..."
BuildAppImageFiles
echo "Done."
echo;

echo "Builds are ready for distribution and usage!"

