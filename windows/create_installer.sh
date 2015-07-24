#!/bin/sh

. ./variables.sh

make -k install DESTDIR=tmp_dir

mkdir "$INSTALLER_DIR"
cp -r tmp_dir/`readlink -f $INSTALL_DIR`/*                "$INSTALLER_DIR"
rm -rf tmp_dir

strip "$INSTALLER_DIR/eiskaltdcpp-qt.exe"
strip "$INSTALLER_DIR/eiskaltdcpp-daemon.exe"

cp $SOURCES_DIR/windows/dcppboot.xml                      "$INSTALLER_DIR"
cp $SOURCES_DIR/eiskaltdcpp-cli/cli-jsonrpc-config.pl     "$INSTALLER_DIR"
echo [Paths] > "$INSTALLER_DIR/qt.conf"
echo Plugins = ./plugins >> "$INSTALLER_DIR/qt.conf"

cp "$QT_MINGW32_DIR/bin/QtCore4.dll"                      "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtGui4.dll"                       "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtNetwork4.dll"                   "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtXml4.dll"                       "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtScript4.dll"                    "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtDeclarative4.dll"               "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtSql4.dll"                       "$INSTALLER_DIR"
cp "$QT_MINGW32_DIR/bin/QtXmlPatterns4.dll"               "$INSTALLER_DIR"

mkdir -p "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_core.dll"        "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_gui.dll"         "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_network.dll"     "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_opengl.dll"      "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_phonon.dll"      "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_sql.dll"         "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_svg.dll"         "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_uitools.dll"     "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_webkit.dll"      "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_xml.dll"         "$INSTALLER_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_xmlpatterns.dll" "$INSTALLER_DIR/script/"

cp "$QT_MINGW32_DIR/bin/libgcc_s_dw2-1.dll"               "$INSTALLER_DIR"
cp "/usr/lib/gcc/$MINGW32_NAME/4.8/libgcc_s_sjlj-1.dll"   "$INSTALLER_DIR"
cp "/usr/lib/gcc/$MINGW32_NAME/4.8/libstdc++-6.dll"       "$INSTALLER_DIR"
cp "/usr/$MINGW32_NAME/lib/libwinpthread-1.dll"           "$INSTALLER_DIR"

cp "$GETTEXT_DIR/bin/libintl-8.dll"                       "$INSTALLER_DIR"
cp "$LIBICONV_DIR/bin/libiconv-2.dll"                     "$INSTALLER_DIR"
cp "$LIBZ_DIR/bin/libz-1.dll"                             "$INSTALLER_DIR"
cp "$LIBBZ2_DIR/bin/libbz2-2.dll"                         "$INSTALLER_DIR"
cp "$LUA_DIR/bin/lua51.dll"                               "$INSTALLER_DIR"
cp "$LIBIDN_DIR/bin/libidn-11.dll"                        "$INSTALLER_DIR"
cp "$ASPELL_DIR/bin/libaspell-15.dll"                     "$INSTALLER_DIR"
cp "$PCRE_DIR/bin/libpcre-0.dll"                          "$INSTALLER_DIR"
cp "$PCRE_DIR/bin/libpcrecpp-0.dll"                       "$INSTALLER_DIR"

mkdir -p "$INSTALLER_DIR/aspell/data/"
mkdir -p "$INSTALLER_DIR/aspell/dict/"
cp $ASPELL_DIR/lib/aspell-0.60/*                          "$INSTALLER_DIR/aspell/data/"

cp "$OPENSSL_DIR/bin/ssleay32.dll"                        "$INSTALLER_DIR"
cp "$OPENSSL_DIR/bin/libeay32.dll"                        "$INSTALLER_DIR"

mkdir -p "$INSTALLER_DIR/plugins/sqldrivers/"
cp "$QT_MINGW32_DIR/plugins/sqldrivers/qsqlite4.dll"      "$INSTALLER_DIR/plugins/sqldrivers/"

cp "$SOURCES_DIR/data/icons/eiskaltdcpp.ico"              "$INSTALLER_DIR"
cp "$SOURCES_DIR/data/icons/icon_164x314.bmp"             "$INSTALLER_DIR"
cp "$SOURCES_DIR/LICENSE"                                 "$INSTALLER_DIR"

makensis ./EiskaltDC++.nsi
