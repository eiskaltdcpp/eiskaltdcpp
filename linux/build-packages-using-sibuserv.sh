#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2019-05-26
# Version: N/A
#
# Dependencies:
# git, wget, curl, rsync, find, sed, tar
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

TestInternetConnection
PrepareMainDir

echo "Getting the sources..."
echo;

GetProgramSources
GetWebUISources
GetProgramVersion

echo "Preparing to build..."
PrepareToBuildForLinux
CleanBuildDir
echo "Done."
echo;

echo "Building EiskaltDC++..."
BuildProject
echo;

echo "Copying programs, libraries, resources and documentation to..."
InstallAllToTempDirForLinux
echo;

echo "Compressing directories into *.tar.xz archives..."
CompressDirsForLinux
echo "Done."
echo;

echo "Moving tarballs to main directory..."
MoveTarballs
echo "Done."
echo;

echo "Builds are ready for distribution and usage!"

