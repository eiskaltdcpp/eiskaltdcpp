#!/bin/sh

. ./variables.sh

make -k install

strip "$INSTALL_DIR/eiskaltdcpp-qt.exe"
strip "$INSTALL_DIR/eiskaltdcpp-daemon.exe"

cp $SOURCES_DIR/win32/dcppboot.xml                        "$INSTALL_DIR"
echo [Paths] > "$INSTALL_DIR/qt.conf"
echo Plugins = ./plugins >> "$INSTALL_DIR/qt.conf"

cp "$QT_MINGW32_DIR/bin/QtCore4.dll"                      "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtGui4.dll"                       "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtNetwork4.dll"                   "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtXml4.dll"                       "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtScript4.dll"                    "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtDeclarative4.dll"               "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtSql4.dll"                       "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/QtXmlPatterns4.dll"               "$INSTALL_DIR"

mkdir -p "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_core.dll"        "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_gui.dll"         "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_network.dll"     "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_opengl.dll"      "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_phonon.dll"      "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_sql.dll"         "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_svg.dll"         "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_uitools.dll"     "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_webkit.dll"      "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_xml.dll"         "$INSTALL_DIR/script/"
cp "$MINGW32_DEPENDS_DIR/script/qtscript_xmlpatterns.dll" "$INSTALL_DIR/script/"

cp "$QT_MINGW32_DIR/bin/libgcc_s_dw2-1.dll"               "$INSTALL_DIR"
#cp "$MINGW32_DIR/bin/libgcc_s_sjlj-1.dll"                 "$INSTALL_DIR"
#cp "$MINGW32_DIR/bin/libstdc++-6.dll"                     "$INSTALL_DIR"
cp "/usr/lib/gcc/$MINGW32_NAME/4.6/libgcc_s_sjlj-1.dll"   "$INSTALL_DIR"
cp "/usr/lib/gcc/$MINGW32_NAME/4.6/libstdc++-6.dll"       "$INSTALL_DIR"
cp "$QT_MINGW32_DIR/bin/mingwm10.dll"                     "$INSTALL_DIR"

cp "$GETTEXT_DIR/bin/libintl-8.dll"                       "$INSTALL_DIR"
cp "$LIBICONV_DIR/bin/libiconv-2.dll"                     "$INSTALL_DIR"
cp "$LIBZ_DIR/bin/libz-1.dll"                             "$INSTALL_DIR"
cp "$LIBBZ2_DIR/bin/libbz2-2.dll"                         "$INSTALL_DIR"
cp "$LUA_DIR/bin/lua51.dll"                               "$INSTALL_DIR"
cp "$LIBIDN_DIR/bin/libidn-11.dll"                        "$INSTALL_DIR"
cp "$ASPELL_DIR/bin/libaspell-15.dll"                     "$INSTALL_DIR"
cp "$PCRE_DIR/bin/libpcre-0.dll"                          "$INSTALL_DIR"
cp "$PCRE_DIR/bin/libpcrecpp-0.dll"                       "$INSTALL_DIR"

mkdir -p "$INSTALL_DIR/aspell/data/"
mkdir -p "$INSTALL_DIR/aspell/dict/"
cp $ASPELL_DIR/lib/aspell-0.60/*                          "$INSTALL_DIR/aspell/data/"

cp "$OPENSSL_DIR/bin/ssleay32.dll"                        "$INSTALL_DIR"
cp "$OPENSSL_DIR/bin/libeay32.dll"                        "$INSTALL_DIR"

mkdir -p "$INSTALL_DIR/plugins/sqldrivers/"
cp "$QT_MINGW32_DIR/plugins/sqldrivers/qsqlite4.dll"      "$INSTALL_DIR/plugins/sqldrivers/"
