#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: MIT (Expat)
# Created: 2019-04-01
# Updated: 2019-04-28
# Version: N/A
#
# Dependencies:
# git

set -e

EISKALTDCPP_DIR_NAME="eiskaltdcpp"
WEB_UI_DIR_NAME="eiskaltdcpp-web"

EISKALTDCPP_URL="https://github.com/eiskaltdcpp/eiskaltdcpp.git"
WEB_UI_URL="https://github.com/eiskaltdcpp/eiskaltdcpp-web.git"

TestInternetConnection()
{
    echo "Checking Internet connection..."
    host github.com 2>&1 > /dev/null && return 0 || return 1
    echo "Done."
    echo;
}

GetProgramSources()
{
    [ -z "${MAIN_DIR}" ] && return 1
    cd "${MAIN_DIR}"

    MOD="${EISKALTDCPP_DIR_NAME}"
    URL="${EISKALTDCPP_URL}"
    if [ -d "${MAIN_DIR}/${MOD}" ]; then
        echo "Updating ${MAIN_DIR}/${MOD}"
        cd "${MAIN_DIR}/${MOD}"
        git checkout .
        git checkout master
        git pull --all --prune -f
        echo;
    else
        echo "Creating ${MAIN_DIR}/${MOD}"
        cd "${MAIN_DIR}"
        git clone "${URL}"
        cd "${MAIN_DIR}/${MOD}"
        git checkout master
        echo;
    fi
}

GetWebUISources()
{
    [ -z "${MAIN_DIR}" ] && return 1
    cd "${MAIN_DIR}"

    MOD="${WEB_UI_DIR_NAME}"
    URL="${WEB_UI_URL}"
    if [ -d "${MAIN_DIR}/${MOD}" ]; then
        echo "Updating ${MAIN_DIR}/${MOD}"
        cd "${MAIN_DIR}/${MOD}"
        git checkout .
        git checkout master
        git pull --all --prune -f
        echo;
    else
        echo "Creating ${MAIN_DIR}/${MOD}"
        cd "${MAIN_DIR}"
        git clone "${URL}"
        cd "${MAIN_DIR}/${MOD}"
        git checkout master
        echo;
    fi
}

