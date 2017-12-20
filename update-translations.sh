#! /bin/bash

# Author:  Boris Pek <tehnick-8@yandex.ru>
# License: Public Domain
# Created: 2011-11-26
# Updated: 2017-07-26
# Version: N/A

set -e

export CUR_DIR="$(dirname $(realpath -s ${0}))"
export MAIN_DIR="${CUR_DIR}/.."
export LANG_DIR="${MAIN_DIR}/eiskaltdcpp_transifex"

cd "${CUR_DIR}"

case "${1}" in
"up")

    git pull --all

;;
"cm")

    git commit -a -m 'Translations are updated from Transifex.'

;;
"make")

    if [ -d "${CUR_DIR}/builddir" ]; then
        cd "${CUR_DIR}/builddir"
        make eiskaltdcpp-qt_tr mo-update
    else
        mkdir -p builddir && cd builddir
        cmake -DUSE_QT=ON -DUSE_GTK=ON ..
        make eiskaltdcpp-qt_tr mo-update
    fi

;;
"tr")

    # Test Internet connection:
    host transifex.com > /dev/null

    git status

    if [ ! -d "${LANG_DIR}" ]; then
        "${0}" tr_co
    fi

    cd "${LANG_DIR}"
    tx pull

    cd "${LANG_DIR}/translations/eiskaltdcpp.dcpp"
    cp *.po "${CUR_DIR}/dcpp/po/"

    cd "${LANG_DIR}/translations/eiskaltdcpp.eiskaltdcpp-gtk"
    cp *.po "${CUR_DIR}/eiskaltdcpp-gtk/po/"

    cd "${LANG_DIR}/translations/eiskaltdcpp.eiskaltdcpp-qt"
    cp *.ts "${CUR_DIR}/eiskaltdcpp-qt/translations/"

    cd "${CUR_DIR}"
    git status

;;
"tr_up")

    cd "${CUR_DIR}"

    export QT_SELECT=qt4
    lupdate eiskaltdcpp-qt/translations.pro

    cd builddir && make pot-update
    cp -fa dcpp/po/libeiskaltdcpp.pot dcpp/po/en.po
    cp -fa eiskaltdcpp-gtk/po/eiskaltdcpp-gtk.pot eiskaltdcpp-gtk/po/en.po

    cd "${CUR_DIR}"
    git status

;;
"tr_cl")

    cd "${CUR_DIR}"

    export QT_SELECT=qt4
    lupdate -verbose -no-obsolete eiskaltdcpp-qt/translations.pro

    cd builddir && make pot-update
    cp -fa dcpp/po/libeiskaltdcpp.pot dcpp/po/en.po
    cp -fa eiskaltdcpp-gtk/po/eiskaltdcpp-gtk.pot eiskaltdcpp-gtk/po/en.po

    cd "${CUR_DIR}"
    git status

;;
"tr_push")

    cd "${LANG_DIR}/translations"

    cp "${CUR_DIR}"/dcpp/po/*.po eiskaltdcpp.dcpp/
    cp "${CUR_DIR}"/eiskaltdcpp-gtk/po/*.po eiskaltdcpp.eiskaltdcpp-gtk/
    cp "${CUR_DIR}"/eiskaltdcpp-qt/translations/*.ts eiskaltdcpp.eiskaltdcpp-qt/

    cd "${LANG_DIR}"
    if [ -z "${2}" ]; then
        echo "<arg> is not specified!" ; exit 1
    elif [ "${2}" = "src" ] ; then
        tx push -s
    elif [ "${2}" = "all" ] ; then
        tx push -s -t --skip
    else
        tx push -t -l ${2} --skip
    fi

;;
"tr_co")

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
*)

    echo "Usage:"
    echo "  up cm make"
    echo "  tr tr_up tr_cl tr_co"
    echo "  tr_push <arg> (arg: src, all or language code)"
    echo ;
    echo "Examples:"
    echo "  ./update-translations.sh tr_push src"
    echo "  ./update-translations.sh tr_push all"
    echo "  ./update-translations.sh tr_push ru"

;;
esac

exit 0
