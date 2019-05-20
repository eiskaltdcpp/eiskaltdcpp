#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2019-05-21
# Version: N/A
#
# Dependencies:
# git, wget, curl, rsync, find, sed, p7zip, nsis
# Sibuserv: https://github.com/sibuserv/sibuserv
# MXE: https://github.com/sibuserv/mxe/tree/hobby

set -e

export MAIN_DIR="${HOME}/Tmp/EiskaltDC++"

CUR_DIR="$(dirname $(realpath -s ${0}))"
. "${CUR_DIR}/downloads_library.sh"
. "${CUR_DIR}/common_functions.sh"

#BUILD_TARGETS="i686-w64-mingw32.shared x86_64-w64-mingw32.shared"
BUILD_TARGETS="i686-w64-mingw32.static x86_64-w64-mingw32.static"

# Script body

TestInternetConnection
PrepareMainDir

echo "Getting the sources..."
echo;

GetProgramSources
GetWebUISources
GetProgramVersion

echo "Preparing to build..."
PrepareToBuild
CleanBuildDir
echo "Done."
echo;

echo "Building EiskaltDC++..."
BuildProjectForWindows
echo;

echo "Copying programs, libraries, resources and documentation to..."
InstallAllToTempDir
echo;

echo "Copying the results to main directory..."
CopyFinalResults
echo "Done."
echo;

echo "Compressing directories into 7z archives..."
CompressDirs
echo "Done."
echo;

echo "Making installers..."
MakeInstallers
echo "Done."
echo;

echo "Moving installers to main directory..."
MoveInstallers
echo "Done."
echo;

echo "Builds are ready for distribution and usage!"

