#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2020-07-29
# Version: N/A
#
# Dependencies:
# git, rsync, find, sed, p7zip, nsis

set -e

P7ZIP_ARCHIVER_OPTIONS="a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"

PROGRAM_VERSION="x.y.z"
WEB_UI_VERSION="x.y.z"

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
        PROGRAM_VERSION="${2}"
    else
        GIT_TAG="$(git describe --tags | cut -d - -f1 | sed "s|v||g")"
        GIT_REV="$(git describe --tags | cut -d - -f2)"
        PROGRAM_VERSION="${GIT_TAG}-${GIT_REV}"
    fi

    ARCHIVE_DIR_NAME="EiskaltDC++-${PROGRAM_VERSION}"
    echo "Current version of EiskaltDC++: ${PROGRAM_VERSION}"
    echo;
}

GetWebUIVersion()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${WEB_UI_DIR_NAME}"
    GIT_TAG="$(git describe --tags | cut -d - -f1 | sed "s|v||g")"
    GIT_REV="$(git describe --tags | cut -d - -f2)"
    WEB_UI_VERSION="${GIT_TAG}-${GIT_REV}"
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

BuildProjectUsingSibuserv()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    build-project ${BUILD_TARGETS}
}

PrepareWebUIToInstallation()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${WEB_UI_DIR_NAME}"
    build-project ${BUILD_TARGETS}
}

InstallToTempDir()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1

    cd "${MAIN_DIR}/${PROJECT_DIR_NAME}"
    build-project install ${BUILD_TARGETS}
}

InstallAllToTempDir()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${WEB_UI_DIR_NAME}" ] && return 1

    InstallToTempDir

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

MakeInstallers()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${PROGRAM_VERSION}" ] && return 1

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
            makensis -Dversion=${PROGRAM_VERSION} -Dshared=32 \
                     ./EiskaltDC++.nsi > /dev/null
        elif [ "${TARGET}" = "i686-w64-mingw32.static" ] ; then
            makensis -Dversion=${PROGRAM_VERSION} -Dstatic=32 \
                     ./EiskaltDC++.nsi > /dev/null
        elif [ "${TARGET}" = "x86_64-w64-mingw32.shared" ] ; then
            makensis -Dversion=${PROGRAM_VERSION} -Dshared=64 \
                     ./EiskaltDC++.nsi > /dev/null
        elif [ "${TARGET}" = "x86_64-w64-mingw32.static" ] ; then
            makensis -Dversion=${PROGRAM_VERSION} -Dstatic=64 \
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

PrepareAppDirs()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${BUILD_TARGETS}" ] && return 1
    [ -z "${PROJECT_DIR_NAME}" ] && return 1
    [ -z "${PROGRAM_VERSION}" ] && return 1

    cd "${MAIN_DIR}"
    rm -rf eiskaltdcpp-*-${PROGRAM_VERSION}*/

    for TARGET in ${BUILD_TARGETS} ; do
        DIR_IN="${MAIN_DIR}/build-${PROJECT_DIR_NAME}/${TARGET}-out/usr"
        if [ "${TARGET}" = "Ubuntu-14.04_i386_shared" ] ; then
            DIR_OUT_QT_UI="eiskaltdcpp-qt-${PROGRAM_VERSION}-i686/usr"
            DIR_OUT_DAEMON="eiskaltdcpp-daemon-${PROGRAM_VERSION}-i686/usr"
        elif [ "${TARGET}" = "Ubuntu-14.04_i386_static" ] ; then
            DIR_OUT_QT_UI="eiskaltdcpp-qt-${PROGRAM_VERSION}-i686/usr"
            DIR_OUT_DAEMON="eiskaltdcpp-daemon-${PROGRAM_VERSION}-i686/usr"
        elif [ "${TARGET}" = "Ubuntu-14.04_amd64_shared" ] ; then
            DIR_OUT_QT_UI="eiskaltdcpp-qt-${PROGRAM_VERSION}-x86_64/usr"
            DIR_OUT_DAEMON="eiskaltdcpp-daemon-${PROGRAM_VERSION}-x86_64/usr"
        elif [ "${TARGET}" = "Ubuntu-14.04_amd64_static" ] ; then
            DIR_OUT_QT_UI="eiskaltdcpp-qt-${PROGRAM_VERSION}-x86_64/usr"
            DIR_OUT_DAEMON="eiskaltdcpp-daemon-${PROGRAM_VERSION}-x86_64/usr"
        else
            continue
        fi

        # eiskaltdcpp-qt dirs tree
        mkdir -p "${DIR_OUT_QT_UI}/bin"
        mkdir -p "${DIR_OUT_QT_UI}/share/applications"
        mkdir -p "${DIR_OUT_QT_UI}/share/man/man1"
        mkdir -p "${DIR_OUT_QT_UI}/share/metainfo"
        # basic files
        cp -a "${DIR_IN}/bin/eiskaltdcpp-qt" \
              "${DIR_OUT_QT_UI}/bin/"
        cp -a "${DIR_IN}/share/applications/eiskaltdcpp-qt.desktop" \
              "${DIR_OUT_QT_UI}/share/applications/"
        cp -a "${DIR_IN}/share/locale" \
              "${DIR_OUT_QT_UI}/share/"
        cp -a "${DIR_IN}/share/man/man1/eiskaltdcpp-qt.1.gz" \
              "${DIR_OUT_QT_UI}/share/man/man1/"
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/eiskaltdcpp-qt/eiskaltdcpp-qt.appdata.xml" \
              "${DIR_OUT_QT_UI}/share/metainfo/eiskaltdcpp-qt.appdata.xml"
        cp -a "${DIR_IN}/share/pixmaps" \
              "${DIR_OUT_QT_UI}/share/"
        # additional files
        cp -a "${DIR_IN}/share/eiskaltdcpp" \
              "${DIR_OUT_QT_UI}/share/"
        cp -a "${DIR_IN}/share/icons" \
              "${DIR_OUT_QT_UI}/share/"
        # AppDir files
        ln -s "usr/bin/eiskaltdcpp-qt" \
              "${DIR_OUT_QT_UI}/../AppRun"
        ln -s "usr/share/applications/eiskaltdcpp-qt.desktop" \
              "${DIR_OUT_QT_UI}/../eiskaltdcpp-qt.desktop"
        ln -s "usr/share/pixmaps/eiskaltdcpp.png" \
              "${DIR_OUT_QT_UI}/../eiskaltdcpp.png"
        ln -s "usr/share/pixmaps/eiskaltdcpp.png" \
              "${DIR_OUT_QT_UI}/../.DirIcon"

        # eiskaltdcpp-daemon dirs tree
        mkdir -p "${DIR_OUT_DAEMON}/bin"
        mkdir -p "${DIR_OUT_DAEMON}/share/applications"
        mkdir -p "${DIR_OUT_DAEMON}/share/eiskaltdcpp"
        mkdir -p "${DIR_OUT_DAEMON}/share/man/man1"
        mkdir -p "${DIR_OUT_DAEMON}/share/metainfo"
        # basic files
        cp -a "${DIR_IN}/bin/eiskaltdcpp-daemon" \
              "${DIR_OUT_DAEMON}/bin/"
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/eiskaltdcpp-daemon/eiskaltdcpp-daemon.desktop" \
              "${DIR_OUT_DAEMON}/share/applications/"
        cp -a "${DIR_IN}/share/locale" \
              "${DIR_OUT_DAEMON}/share/"
        cp -a "${DIR_IN}/share/man/man1/eiskaltdcpp-daemon.1.gz" \
              "${DIR_OUT_DAEMON}/share/man/man1/"
        cp -a "${MAIN_DIR}/${PROJECT_DIR_NAME}/eiskaltdcpp-qt/eiskaltdcpp-qt.appdata.xml" \
              "${DIR_OUT_DAEMON}/share/metainfo/eiskaltdcpp-daemon.appdata.xml"
        cp -a "${DIR_IN}/share/pixmaps" \
              "${DIR_OUT_DAEMON}/share/"
        # additional files
        cp -a "${DIR_IN}/share/eiskaltdcpp/luascripts" \
              "${DIR_OUT_DAEMON}/share/eiskaltdcpp/"
        # AppDir files
        ln -s "usr/bin/eiskaltdcpp-daemon" \
              "${DIR_OUT_DAEMON}/../AppRun"
        ln -s "usr/share/applications/eiskaltdcpp-daemon.desktop" \
              "${DIR_OUT_DAEMON}/../eiskaltdcpp-daemon.desktop"
        ln -s "usr/share/pixmaps/eiskaltdcpp.png" \
              "${DIR_OUT_DAEMON}/../eiskaltdcpp.png"
        ln -s "usr/share/pixmaps/eiskaltdcpp.png" \
              "${DIR_OUT_DAEMON}/../.DirIcon"
    done
}

BuildAppImageFiles()
{
    [ -z "${MAIN_DIR}" ] && return 1
    [ -z "${PROGRAM_VERSION}" ] && return 1

    cd "${MAIN_DIR}"
    rm -f eiskaltdcpp-*-${PROGRAM_VERSION}*.AppImage

    for DIR in eiskaltdcpp-*-${PROGRAM_VERSION}* ; do
        [ ! -d "${DIR}" ] && continue

        case "${DIR}" in
        *x86_64*)
            APPIMAGETOOL="appimagetool-x86_64.AppImage"
        ;;
        *i686*)
            APPIMAGETOOL="appimagetool-i686.AppImage"
        ;;
        *)
            APPIMAGETOOL="appimagetool"
        esac

        echo "Creating: ${DIR}.AppImage"
        "${APPIMAGETOOL}" "${DIR}" "${DIR}.AppImage" 2>&1 > appimagetool.log
    done
}

