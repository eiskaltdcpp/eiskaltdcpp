#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2020-06-17
# Version: N/A
#
# Dependencies:
# git, wget, curl, rsync, find, sed, p7zip, nsis

set -e

P7ZIP_ARCHIVER_OPTIONS="a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
TAR_ARCHIVER_OPTIONS="-cJf"

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
    [ -z "${PROJECT_DIR_NAME}" ] && return 1

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
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}"
    rm -rf "${MAIN_DIR}/build-${PROJECT_DIR_NAME}"
    rm -rf "${MAIN_DIR}/build-${WEB_UI_DIR_NAME}"
    rm -rf "${MAIN_DIR}/${PROJECT_DIR_NAME}/builddir"
}

PrepareToBuildForWindows()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    sed -i "s|option (USE_JS .*$|option (USE_JS \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (DBUS_NOTIFY .*$|option (DBUS_NOTIFY \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (WITH_EXAMPLES .*$|option (WITH_EXAMPLES \"\" OFF)|g" CMakeLists.txt
    sed -i "s|option (NO_UI_DAEMON .*$|option (NO_UI_DAEMON \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (JSONRPC_DAEMON .*$|option (JSONRPC_DAEMON \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (WITH_LUASCRIPTS .*$|option (WITH_LUASCRIPTS \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (INSTALL_DEPENDENCIES .*$|option (INSTALL_DEPENDENCIES \"\" ON)|g" CMakeLists.txt
}

PrepareToBuildForLinux()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    sed -i "s|option (WITH_EXAMPLES .*$|option (WITH_EXAMPLES \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (NO_UI_DAEMON .*$|option (NO_UI_DAEMON \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (JSONRPC_DAEMON .*$|option (JSONRPC_DAEMON \"\" ON)|g" CMakeLists.txt
    sed -i "s|option (WITH_LUASCRIPTS .*$|option (WITH_LUASCRIPTS \"\" ON)|g" CMakeLists.txt
}

BuildProject()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    build-project ${BUILD_TARGETS}

    cd "${MAIN_DIR}/${WEB_UI_DIR_NAME}"
    build-project ${BUILD_TARGETS}
}

InstallAllToTempDir()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    build-project install ${BUILD_TARGETS}

    cd "${MAIN_DIR}/${WEB_UI_DIR_NAME}"
    build-project install ${BUILD_TARGETS}

    for TARGET in ${BUILD_TARGETS} ; do
        DIR_SRC="${MAIN_DIR}/build-${WEB_UI_DIR_NAME}/${TARGET}-out/usr"
        DIR_OUT="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out/usr"

        cp -af "${DIR_SRC}"/* "${DIR_OUT}/"
    done
}

CopyFinalResultsForWindows()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}"
    for TARGET in ${BUILD_TARGETS} ; do
        DIR_IN="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out/usr"
        if [ "${TARGET}" = "i686-w64-mingw32.shared" ] ; then
            DIR_OUT="${ARCHIVE_DIR_NAME}_x86"
        elif [ "${TARGET}" = "i686-w64-mingw32.static" ] ; then
            DIR_OUT="${ARCHIVE_DIR_NAME}_x86"
        elif [ "${TARGET}" = "x86_64-w64-mingw32.shared" ] ; then
            DIR_OUT="${ARCHIVE_DIR_NAME}_x86_64"
        elif [ "${TARGET}" = "x86_64-w64-mingw32.static" ] ; then
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

CompressDirsForWindows()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1
    [ -z "${P7ZIP_ARCHIVER_OPTIONS}" ] && return 1

    cd "${MAIN_DIR}"
    rm -f ${ARCHIVE_DIR_NAME}*-portable.7z
    for DIR in ${ARCHIVE_DIR_NAME}* ; do
        [ ! -d "${DIR}" ] && continue

        echo "Creating archive: ${DIR}-portable.7z"
        7z ${P7ZIP_ARCHIVER_OPTIONS} "${DIR}-portable.7z" "${DIR}" > /dev/null
    done
}

CompressDirsForLinux()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1
    [ -z "${TAR_ARCHIVER_OPTIONS}" ] && return 1

    cd "${MAIN_DIR}"
    for TARGET in ${BUILD_TARGETS} ; do
        WORK_DIR="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out"

        TARBALL_DIR_NAME="${ARCHIVE_DIR_NAME}_${TARGET}"
        TARBALL_DIR_NAME="$(echo ${TARBALL_DIR_NAME} | sed 's|Debian-9|linux|')"
        TARBALL_DIR_NAME="$(echo ${TARBALL_DIR_NAME} | sed 's|Ubuntu-14.04|linux|')"
        TARBALL_DIR_NAME="$(echo ${TARBALL_DIR_NAME} | sed 's|_shared||')"
        TARBALL_DIR_NAME="$(echo ${TARBALL_DIR_NAME} | sed 's|_static||')"

        cd "${WORK_DIR}"
        rm -rf "${TARBALL_DIR_NAME}"*

        mkdir "${TARBALL_DIR_NAME}"
        cp -af "usr" "${TARBALL_DIR_NAME}/"

        tar ${TAR_ARCHIVER_OPTIONS} "${TARBALL_DIR_NAME}.tar.xz" "${TARBALL_DIR_NAME}"
    done
}

MakeInstallers()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${VERSION}" ] && return 1

    cd "${MAIN_DIR}"
    for TARGET in ${BUILD_TARGETS} ; do
        WORK_DIR="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out"
        cd "${WORK_DIR}"
        rm -rf installer *.exe EiskaltDC++.nsi
        cp -af "${MAIN_DIR}/${PROJECT_DIR_NAME}/windows/EiskaltDC++.nsi" ./

        mkdir "installer"
        rsync -a "usr/" "installer/" > /dev/null
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/windows/dcppboot.xml" \
              "installer/"

        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/data/icons/eiskaltdcpp.ico" \
              "installer/"
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/data/icons/icon_164x314.bmp" \
              "installer/"
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/LICENSE" \
              "installer/"

        if [ "${TARGET}" = "i686-w64-mingw32.shared" ] ; then
            makensis -Dversion=${VERSION} -Dshared=32 \
                     ./EiskaltDC++.nsi > /dev/null
        elif [ "${TARGET}" = "i686-w64-mingw32.static" ] ; then
            makensis -Dversion=${VERSION} -Dstatic=32 \
                     ./EiskaltDC++.nsi > /dev/null
        elif [ "${TARGET}" = "x86_64-w64-mingw32.shared" ] ; then
            makensis -Dversion=${VERSION} -Dshared=64 \
                     ./EiskaltDC++.nsi > /dev/null
        elif [ "${TARGET}" = "x86_64-w64-mingw32.static" ] ; then
            makensis -Dversion=${VERSION} -Dstatic=64 \
                     ./EiskaltDC++.nsi > /dev/null
        else
            continue
        fi
    done
}

MoveInstallers()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}"
    rm -f ${ARCHIVE_DIR_NAME}*-installer.exe

    cd "${MAIN_DIR}/build-${PROJECT_DIR_NAME}"
    mv */*-installer.exe "${MAIN_DIR}/"
}

MoveTarballs()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${ARCHIVE_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}"
    rm -f ${ARCHIVE_DIR_NAME}*.tar.xz

    cd "${MAIN_DIR}/build-${PROJECT_DIR_NAME}"
    mv */*.tar.xz "${MAIN_DIR}/"
}

