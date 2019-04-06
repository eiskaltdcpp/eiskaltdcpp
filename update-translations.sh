#!/bin/sh

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: Public Domain
# Created: 2011-11-26
# Updated: 2019-04-07
# Version: N/A

set -e

export CUR_DIR="$(dirname $(realpath -s ${0}))"
export MAIN_DIR="$(realpath -s ${CUR_DIR}/..)"
export LANG_DIR="${MAIN_DIR}/eiskaltdcpp_transifex"

cd "${CUR_DIR}"

case "${1}" in
"up")
    # Pulling changes from GitHub repo.

    git pull --all

;;
"cm")
    # Creating correct git commit.

    git commit -a -m 'Translations were updated from Transifex.'

;;
"make")
    # Making precompiled localization files.

    if [ -d "${CUR_DIR}/builddir" ]; then
        cd "${CUR_DIR}/builddir"
        make eiskaltdcpp-qt_tr mo-update
    else
        mkdir -p builddir && cd builddir
        cmake -DUSE_QT5=ON -DUSE_GTK=ON ..
        make eiskaltdcpp-qt_tr mo-update
    fi

;;
"tr")
    # Pulling changes from Transifex.

    # Test Internet connection:
    host transifex.com > /dev/null

    git status

    if [ ! -d "${LANG_DIR}" ]; then
        "${0}" tr_co
    fi

    cd "${LANG_DIR}"
    tx pull

    cd "${LANG_DIR}/translations/eiskaltdcpp.libeiskaltdcpp"
    cp *.po "${CUR_DIR}/dcpp/po/"

    cd "${LANG_DIR}/translations/eiskaltdcpp.eiskaltdcpp-gtk"
    cp *.po "${CUR_DIR}/eiskaltdcpp-gtk/po/"

    cd "${LANG_DIR}/translations/eiskaltdcpp.eiskaltdcpp-qt"
    cp *.ts "${CUR_DIR}/eiskaltdcpp-qt/translations/"

    cd "${LANG_DIR}/translations/eiskaltdcpp.desktop-file"
    cp *.desktop "${CUR_DIR}/eiskaltdcpp-qt/desktop-file/"

    cd "${CUR_DIR}"
    git status

;;
"tr_up")
    # Full update of localization files.

    cd "${CUR_DIR}"

    lupdate eiskaltdcpp-qt/translations.pro

    cd builddir
    make pot-update

    cd "${CUR_DIR}"
    mv -f dcpp/po/libeiskaltdcpp.pot dcpp/po/en.po
    mv -f eiskaltdcpp-gtk/po/eiskaltdcpp-gtk.pot eiskaltdcpp-gtk/po/en.po

    cd "${CUR_DIR}"
    git status

;;
"tr_cl")
    # Cleaning update of localization files.

    cd "${CUR_DIR}"

    lupdate -verbose -no-obsolete eiskaltdcpp-qt/translations.pro

    cd builddir
    make pot-update

    cd "${CUR_DIR}"
    mv -f dcpp/po/libeiskaltdcpp.pot dcpp/po/en.po
    mv -f eiskaltdcpp-gtk/po/eiskaltdcpp-gtk.pot eiskaltdcpp-gtk/po/en.po

    cd "${CUR_DIR}"
    git status

;;
"tr_push")
    # Pushing changes to Transifex.

    cd "${LANG_DIR}/translations"

    cp "${CUR_DIR}"/dcpp/po/*.po eiskaltdcpp.libeiskaltdcpp/
    cp "${CUR_DIR}"/eiskaltdcpp-gtk/po/*.po eiskaltdcpp.eiskaltdcpp-gtk/
    cp "${CUR_DIR}"/eiskaltdcpp-qt/translations/*.ts eiskaltdcpp.eiskaltdcpp-qt/
    cp "${CUR_DIR}"/eiskaltdcpp-qt/desktop-file/*.desktop eiskaltdcpp.desktop-file/

    cd "${LANG_DIR}"
    if [ -z "${2}" ]; then
        echo "<arg> is not specified!"
        exit 1
    elif [ "${2}" = "src" ] ; then
        tx push -s
    elif [ "${2}" = "all" ] ; then
        tx push -s -t --skip
    else
        tx push -t -l ${2} --skip
    fi

;;
"tr_co")
    # Cloning Transifex repo.

    if [ -d "${LANG_DIR}" ]; then
        echo "\"${LANG_DIR}\" directory already exists!"
        exit 1
    else
        echo "Creating ${LANG_DIR}"
        mkdir -p "${LANG_DIR}/.tx"
        cp "transifex.config" "${LANG_DIR}/.tx/config"
        cd "${LANG_DIR}"
        tx pull -a -s
    fi

;;
"tr_sync")
    # Syncing Transifex and GitHub repos.

    "${0}" up > /dev/null
    "${0}" tr > /dev/null

    if [ "$(git status | grep 'l10n/' | wc -l)" -gt 0 ]; then
        "${0}" cm
        "${0}" push
    fi
    echo ;

;;
"desktop_up")
    # Update main .desktop file

    GENERICNAME_FULL_DATA=$(grep -r "GenericName\[" "${CUR_DIR}/eiskaltdcpp-qt/desktop-file/" | grep -v '/en.desktop:')
    GENERICNAME_FILTERED_DATA=$(echo "${GENERICNAME_FULL_DATA}" | sed -ne 's|^.*/.*.desktop:\(.*\)$|\1|p')
    GENERICNAME_SORTED_DATA=$(echo "${GENERICNAME_FILTERED_DATA}" | sort -uV)

    COMMENT_FULL_DATA=$(grep -r "Comment\[" "${CUR_DIR}/eiskaltdcpp-qt/desktop-file/" | grep -v '/en.desktop:')
    COMMENT_FILTERED_DATA=$(echo "${COMMENT_FULL_DATA}" | sed -ne 's|^.*/.*.desktop:\(.*\)$|\1|p')
    COMMENT_SORTED_DATA=$(echo "${COMMENT_FILTERED_DATA}" | sort -uV)

    DESKTOP_FILE="${CUR_DIR}/eiskaltdcpp-qt/eiskaltdcpp-qt.desktop"
    grep -v "GenericName\[" "${DESKTOP_FILE}" > "${DESKTOP_FILE}.tmp"
    mv -f "${DESKTOP_FILE}.tmp" "${DESKTOP_FILE}"
    grep -v "Comment\[" "${DESKTOP_FILE}" > "${DESKTOP_FILE}.tmp"
    mv -f "${DESKTOP_FILE}.tmp" "${DESKTOP_FILE}"
    echo "${GENERICNAME_SORTED_DATA}" >> "${DESKTOP_FILE}"
    echo "${COMMENT_SORTED_DATA}" >> "${DESKTOP_FILE}"

    # Update .desktop file for English localization
    cp -f "${DESKTOP_FILE}" "${CUR_DIR}/eiskaltdcpp-qt/desktop-file/en.desktop"

;;
*)
    # Help.

    echo "Usage:"
    echo "  up cm make"
    echo "  tr tr_up tr_cl tr_co tr_sync desktop_up"
    echo "  tr_push <arg> (arg: src, all or language code)"
    echo ;
    echo "Examples:"
    echo "  ./update-translations.sh tr_push src"
    echo "  ./update-translations.sh tr_push all"
    echo "  ./update-translations.sh tr_push ru"

;;
esac

exit 0
