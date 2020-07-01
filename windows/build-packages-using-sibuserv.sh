#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2020-07-01
# Version: N/A
#
# Dependencies:
# git, rsync, find, sed, p7zip, nsis
# MXE: https://github.com/sibuserv/mxe/tree/hobby
# Sibuserv: https://github.com/sibuserv/sibuserv
#
# Current versions of used libraries may be found here:
# https://github.com/sibuserv/mxe/blob/hobby/docs/packages.json

set -e

export MAIN_DIR="${HOME}/Tmp/EiskaltDC++"

CUR_DIR="$(dirname $(realpath -s ${0}))"
. "${CUR_DIR}/downloads-library.sh"
. "${CUR_DIR}/common-functions.sh"

#BUILD_TARGETS="i686-w64-mingw32.shared x86_64-w64-mingw32.shared"
BUILD_TARGETS="i686-w64-mingw32.static x86_64-w64-mingw32.static"

# Script body

TestInternetConnection
PrepareMainDir

echo "Getting the sources..."
echo;

GetProgramSources ${@}
GetWebUISources
GetProgramVersion ${@}

echo "Preparing to build..."
PrepareToBuildForWindows
CleanBuildDir
echo "Done."
echo;

echo "Building EiskaltDC++..."
BuildProjectUsingSibuserv
PrepareWebUIToInstallation
echo;

echo "Copying programs, libraries, resources and documentation to..."
InstallAllToTempDir
echo;

echo "Copying the results to main directory..."
CopyFinalResultsForWindows
echo "Done."
echo;

echo "Compressing directories into 7z archives..."
CompressDirsForWindows
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

