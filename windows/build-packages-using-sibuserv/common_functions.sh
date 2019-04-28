#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2019-04-28
# Version: N/A
#
# Dependencies:
# git, wget, curl, rsync, find, sed, p7zip, nsis

set -e

PROJECT_DIR_NAME="eiskaltdcpp"

ARCHIVER_OPTIONS="a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"

VERSION="x.y.z"

ShowHelp()
{
    [ -z "${SCRIPT_NAME}" ] && return 1

    if [ "${1}" = "-h" ] || [ "${1}" = "--help" ]; then
        echo "Usage:"
        echo "   ./${SCRIPT_NAME} [options]"
        echo ;
        echo "Examples:"
        echo "  ./${SCRIPT_NAME}"
        echo "  ./${SCRIPT_NAME} release 2.4.0"
        echo "  ./${SCRIPT_NAME} --help"
        echo;
        exit 0;
    elif [ "${1}" = "release" ]; then
        if [ -z "${2}" ]; then
            echo "Error: release version is not specified!"
            echo;
            exit 1;
        fi
    fi
}

PrepareMainDir()
{
    [ -z "${MAIN_DIR}" ] && return 1

    mkdir -p "${MAIN_DIR}"
    cd "${MAIN_DIR}"
}

GetProgramVersion()
{
    [ -z "${MAIN_DIR}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    if [ "${1}" = "release" ]; then
        VERSION="${2}"
    else
        GIT_TAG="$(git describe --tags | cut -d - -f1 | sed "s|v||g")"
        GIT_REV="$(git describe --tags | cut -d - -f2)"
        VERSION="${GIT_TAG}-${GIT_REV}"
    fi

    ARCHIVE_DIR_NAME="EiskaltDC++-${VERSION}"
    echo "Current version of EiskaltDC++: ${VERSION}"
    echo;
}

CleanBuildDir()
{
    [ -z "${MAIN_DIR}" ] && return 1

    cd "${MAIN_DIR}"
    rm -rf "${MAIN_DIR}/build-${PROJECT_DIR_NAME}"
    rm -rf "${MAIN_DIR}/${PROJECT_DIR_NAME}/builddir"
}

PrepareToBuild()
{
    [ -z "${MAIN_DIR}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    sed -i "s|option (USE_JS .*$|option (USE_JS \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (FORCE_XDG .*$|option (FORCE_XDG \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (DBUS_NOTIFY .*$|option (DBUS_NOTIFY \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (WITH_EXAMPLES .*$|option (WITH_EXAMPLES \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (NO_UI_DAEMON .*$|option (NO_UI_DAEMON \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (JSONRPC_DAEMON .*$|option (JSONRPC_DAEMON \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (WITH_LUASCRIPTS .*$|option (WITH_LUASCRIPTS \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (INSTALL_DEPENDENCIES .*$|option (INSTALL_DEPENDENCIES \"\" ON)|g" CMakeLists.txt
}

PrepareToSecondBuild()
{
    [ -z "${MAIN_DIR}" ] && return 1

    cd "${MAIN_DIR}/build-${PROJECT_DIR_NAME}"
    sed -i "s|CHAT_TYPE:STRING=.*$|CHAT_TYPE:STRING=WEBKIT|g" */CMakeCache.txt
}

BuildProjectForWindows()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    build-project ${BUILD_TARGETS}
}

InstallAllToTempDir()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    build-project install ${BUILD_TARGETS}

    for TARGET in ${BUILD_TARGETS} ; do
        DIR_OUT="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out/usr"

        mkdir -p "${DIR_OUT}/docs"
        cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
        cp -af eiskaltdcpp-qt/man.eiskaltdcpp-qt.html \
               eiskaltdcpp-daemon/man.eiskaltdcpp-daemon.html \
               windows/build-packages-using-sibuserv/Readme.aspell.txt \
               "${DIR_OUT}/docs/"

        mkdir -p "${DIR_OUT}/web-ui"
        cd "${MAIN_DIR}/${WEB_UI_DIR_NAME}"
        cp -af images js config.js favicon.ico style.css \
               index.html README.html windows/help.html \
               "${DIR_OUT}/web-ui/"
    done
}

CopyFinalResults()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}"
    for TARGET in ${BUILD_TARGETS} ; do
        DIR_IN="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out/usr"
        if [ "${TARGET}" = "i686-w64-mingw32.shared" ] ; then
            DIR_OUT="${ARCHIVE_DIR_NAME}_x86"
        elif [ "${TARGET}" = "x86_64-w64-mingw32.shared" ] ; then
            DIR_OUT="${ARCHIVE_DIR_NAME}_x86_64"
        else
            continue
        fi

        mkdir -p "${DIR_OUT}"
        rsync -a --del "${DIR_IN}/" "${DIR_OUT}/" > /dev/null
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/windows/dcppboot-portable.xml" \
              "${DIR_OUT}/dcppboot.xml"
    done
}

CompressDirs()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1
    [ -z "${ARCHIVER_OPTIONS}" ] && return 1

    cd "${MAIN_DIR}"
    rm -f ${ARCHIVE_DIR_NAME}*.7z
    for DIR in ${ARCHIVE_DIR_NAME}* ; do
        [ ! -d "${DIR}" ] && continue

        echo "Creating archive: ${DIR}-portable.7z"
        7z ${ARCHIVER_OPTIONS} "${DIR}-portable.7z" "${DIR}" > /dev/null
    done
}

