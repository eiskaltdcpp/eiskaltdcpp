#! /bin/bash

# Author:  Boris Pek <tehnick-8@mail.ru>
# License: Public Domain
# Created: 2011-11-26
# Updated: 2013-06-25
# Version: N/A

if [[ ${0} =~ ^/.+$ ]]; then
    export CUR_DIR="$(dirname ${0})"
else
    export CUR_DIR="${PWD}/$(dirname ${0})"
fi

export MAIN_DIR="${CUR_DIR}/.."

cd "${CUR_DIR}" || exit 1

case "${1}" in
"up")

    git pull --all || exit 1

;;
"cm")

    git commit -a -m 'Translations were updated from Transifex.' || exit 1

;;
"make")

    if [ -d "${CUR_DIR}/builddir" ]; then
        cd "${CUR_DIR}/builddir" || exit 1
        make translations_qt mo-update || exit 1
        
    else
        mkdir -p builddir && cd builddir || exit 1
        cmake -DUSE_QT=ON -DUSE_GTK=ON .. || exit 1
        "${0}" make || exit 1
    fi

;;
"tr")

    # Test Internet connection:
    host transifex.net > /dev/null || exit 1

    git status || exit 1

    LANG_DIR="${MAIN_DIR}/eiskaltdcpp_transifex"

    cd "${LANG_DIR}" || exit 1
    tx pull -a -s || exit 1

    cd "${LANG_DIR}/translations/eiskaltdcpp.dcpp" || exit 1
    cp *.po "${CUR_DIR}/dcpp/po/" || exit 1

    cd "${LANG_DIR}/translations/eiskaltdcpp.eiskaltdcpp-gtk" || exit 1
    cp *.po "${CUR_DIR}/eiskaltdcpp-gtk/po/" || exit 1

    cd "${LANG_DIR}/translations/eiskaltdcpp.eiskaltdcpp-qt" || exit 1
    cp *.ts "${CUR_DIR}/eiskaltdcpp-qt/translations/" || exit 1

    cd "${CUR_DIR}" || exit 1
    git status || exit 1

;;
"tr_up")

    cd "${CUR_DIR}" || exit 1

    export QT_SELECT=qt4
    lupdate eiskaltdcpp-qt/translations.pro || exit 1

    echo "" | tee dcpp/po/en.po
    echo "" | tee eiskaltdcpp-gtk/po/en.po
    cd builddir && make pot-update || exit 1

    git status || exit 1

;;
"tr_cl")

    cd "${CUR_DIR}" || exit 1

    lupdate -verbose -no-obsolete eiskaltdcpp-qt/translations.pro || exit 1
    cd builddir && make pot-update || exit 1

    git status || exit 1

;;
"tr_push")

    LANG_DIR="${MAIN_DIR}/eiskaltdcpp_transifex"
    cd "${LANG_DIR}/translations" || exit 1

    cp "${CUR_DIR}"/dcpp/po/*.po eiskaltdcpp.dcpp/ || exit 1
    cp "${CUR_DIR}"/eiskaltdcpp-gtk/po/*.po eiskaltdcpp.eiskaltdcpp-gtk/ || exit 1
    cp "${CUR_DIR}"/eiskaltdcpp-qt/translations/*.ts eiskaltdcpp.eiskaltdcpp-qt/ || exit 1

    cd "${LANG_DIR}" || exit 1
    if [ -z "${2}" ]; then
        echo "<arg> is not specified!" ; exit 1
    elif [ "${2}" = "src" ] ; then
        tx push -s || exit 1
    elif [ "${2}" = "all" ] ; then
        tx push -s -t || exit 1
    else
        tx push -t -l ${2} || exit 1
    fi

;;
"tr_co")

    if [ -d "${MAIN_DIR}/eiskaltdcpp_transifex" ]; then
        echo "${MAIN_DIR}/eiskaltdcpp_transifex"
        echo "directory already exists!"
    else
        echo "Creating ${MAIN_DIR}/eiskaltdcpp_transifex"
        mkdir -p "${MAIN_DIR}/eiskaltdcpp_transifex/.tx"
        cp "transifex.config" "${MAIN_DIR}/eiskaltdcpp_transifex/.tx/config" || exit 1
        cd "${MAIN_DIR}/eiskaltdcpp_transifex" || exit 1
        tx pull -a -s || exit 1
    fi

;;
*)

    echo "Usage:"
    echo "  up cm make"
    echo "  tr tr_up tr_cl tr_co"
    echo "  tr_push <arg> (arg: src, all or language)"
    echo ;
    echo "Examples:"
    echo "  ./update-translations.sh tr_push src"
    echo "  ./update-translations.sh tr_push all"
    echo "  ./update-translations.sh tr_push ru"

;;
esac

exit 0
