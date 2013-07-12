call variables.bat

mingw32-make -k install

strip "%INSTALL_DIR%\eiskaltdcpp-qt.exe"
strip "%INSTALL_DIR%\eiskaltdcpp-daemon.exe"

copy /Y %SOURCES_DIR%\win32\dcppboot.xml                        "%INSTALL_DIR%"
copy /Y %SOURCES_DIR%\eiskaltdcpp-cli\cli-jsonrpc-config.pl     "%INSTALL_DIR%"
echo [Paths] > "%INSTALL_DIR%\qt.conf"
echo Plugins = ./plugins >> "%INSTALL_DIR%\qt.conf"

copy /Y "%QT_MINGW32_DIR%\bin\QtCore4.dll"                      "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtGui4.dll"                       "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtNetwork4.dll"                   "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtXml4.dll"                       "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtScript4.dll"                    "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtDeclarative4.dll"               "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtSql4.dll"                       "%INSTALL_DIR%"
copy /Y "%QT_MINGW32_DIR%\bin\QtXmlPatterns4.dll"               "%INSTALL_DIR%"

mkdir "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_core.dll"        "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_gui.dll"         "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_network.dll"     "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_opengl.dll"      "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_phonon.dll"      "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_sql.dll"         "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_svg.dll"         "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_uitools.dll"     "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_webkit.dll"      "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_xml.dll"         "%INSTALL_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_xmlpatterns.dll" "%INSTALL_DIR%\script\"

copy /Y "%MINGW32_DIR%\bin\libstdc++-6.dll"                     "%INSTALL_DIR%"
copy /Y "%MINGW32_DIR%\bin\libgcc_s_dw2-1.dll"                  "%INSTALL_DIR%"
copy /Y "%MINGW32_DIR%\bin\mingwm10.dll"                        "%INSTALL_DIR%"

copy /Y "%GETTEXT_DIR%\bin\libintl-8.dll"                       "%INSTALL_DIR%"
copy /Y "%LIBICONV_DIR%\bin\libiconv-2.dll"                     "%INSTALL_DIR%"
copy /Y "%LIBZ_DIR%\bin\libz-1.dll"                             "%INSTALL_DIR%"
copy /Y "%LIBBZ2_DIR%\bin\libbz2-2.dll"                         "%INSTALL_DIR%"
copy /Y "%LUA_DIR%\bin\lua51.dll"                               "%INSTALL_DIR%"
copy /Y "%LIBIDN_DIR%\bin\libidn-11.dll"                        "%INSTALL_DIR%"
copy /Y "%ASPELL_DIR%\bin\libaspell-15.dll"                     "%INSTALL_DIR%"
copy /Y "%PCRE_DIR%\bin\libpcre-0.dll"                          "%INSTALL_DIR%"
copy /Y "%PCRE_DIR%\bin\libpcrecpp-0.dll"                       "%INSTALL_DIR%"

mkdir "%INSTALL_DIR%\aspell\data\"
mkdir "%INSTALL_DIR%\aspell\dict\"
copy /Y "%ASPELL_DIR%\lib\aspell-0.60\*"                        "%INSTALL_DIR%\aspell\data\"

copy /Y "%OPENSSL_DIR%\bin\ssleay32.dll"                        "%INSTALL_DIR%"
copy /Y "%OPENSSL_DIR%\bin\libeay32.dll"                        "%INSTALL_DIR%"

mkdir "%INSTALL_DIR%\plugins\sqldrivers\"
copy /Y "%QT_MINGW32_DIR%\plugins\sqldrivers\qsqlite4.dll"      "%INSTALL_DIR%\plugins\sqldrivers\"
