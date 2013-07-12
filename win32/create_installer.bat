call variables.bat

mingw32-make -k install DESTDIR=tmp_dir

mkdir "%INSTALLER_DIR%"
set cut_path=%~dp0
set cut_path=%cut_path:~3%
xcopy /E /R /Y /I "tmp_dir\%cut_path%\%INSTALL_DIR%\*"                 %INSTALLER_DIR%
rmdir /s /q tmp_dir

strip "%INSTALLER_DIR%\eiskaltdcpp-qt.exe"
strip "%INSTALLER_DIR%\eiskaltdcpp-daemon.exe"

copy /Y "%SOURCES_DIR%\data\icons\eiskaltdcpp.ico"                     %INSTALLER_DIR%
copy /Y "%SOURCES_DIR%\data\icons\icon_164x314.bmp"                    %INSTALLER_DIR%
copy /Y "%SOURCES_DIR%\win32\dcppboot.xml"                             %INSTALLER_DIR%
copy /Y "%SOURCES_DIR%\eiskaltdcpp-cli\cli-jsonrpc-config.pl"          %INSTALLER_DIR%
copy /Y "%SOURCES_DIR%\LICENSE"                                        %INSTALLER_DIR%
echo [Paths] > "%INSTALLER_DIR%\qt.conf"
echo Plugins = ./plugins >> "%INSTALLER_DIR%\qt.conf"

copy /Y "%QT_MINGW32_DIR%\bin\QtCore4.dll"                             %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtGui4.dll"                              %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtNetwork4.dll"                          %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtXml4.dll"                              %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtScript4.dll"                           %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtDeclarative4.dll"                      %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtSql4.dll"                              %INSTALLER_DIR%
copy /Y "%QT_MINGW32_DIR%\bin\QtXmlPatterns4.dll"                      %INSTALLER_DIR%

mkdir "%INSTALLER_DIR%\script\"
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_core.dll"               %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_gui.dll"                %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_network.dll"            %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_opengl.dll"             %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_phonon.dll"             %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_sql.dll"                %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_svg.dll"                %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_uitools.dll"            %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_webkit.dll"             %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_xml.dll"                %INSTALLER_DIR%\script\
copy /Y "%MINGW32_DEPENDS_DIR%\script\qtscript_xmlpatterns.dll"        %INSTALLER_DIR%\script\

copy /Y "%MINGW32_DIR%\bin\libstdc++-6.dll"                            %INSTALLER_DIR%
copy /Y "%MINGW32_DIR%\bin\libgcc_s_dw2-1.dll"                         %INSTALLER_DIR%
copy /Y "%MINGW32_DIR%\bin\mingwm10.dll"                               %INSTALLER_DIR%

copy /Y "%GETTEXT_DIR%\bin\libintl-8.dll"                              %INSTALLER_DIR%
copy /Y "%LIBICONV_DIR%\bin\libiconv-2.dll"                            %INSTALLER_DIR%
copy /Y "%LIBZ_DIR%\bin\libz-1.dll"                                    %INSTALLER_DIR%
copy /Y "%LIBBZ2_DIR%\bin\libbz2-2.dll"                                %INSTALLER_DIR%
copy /Y "%LUA_DIR%\bin\lua51.dll"                                      %INSTALLER_DIR%
copy /Y "%LIBIDN_DIR%\bin\libidn-11.dll"                               %INSTALLER_DIR%
copy /Y "%ASPELL_DIR%\bin\libaspell-15.dll"                            %INSTALLER_DIR%
copy /Y "%PCRE_DIR%\bin\libpcre-0.dll"                                 %INSTALLER_DIR%
copy /Y "%PCRE_DIR%\bin\libpcrecpp-0.dll"                              %INSTALLER_DIR%

mkdir "%INSTALLER_DIR%\aspell\data\"
mkdir "%INSTALLER_DIR%\aspell\dict\"
copy /Y "%ASPELL_DIR%\lib\aspell-0.60\*"                               %INSTALLER_DIR%\aspell\data\

copy /Y "%OPENSSL_DIR%\bin\ssleay32.dll"                               %INSTALLER_DIR%
copy /Y "%OPENSSL_DIR%\bin\libeay32.dll"                               %INSTALLER_DIR%

mkdir "%INSTALLER_DIR%\plugins\sqldrivers\"
copy /Y "%QT_MINGW32_DIR%\plugins\sqldrivers\qsqlite4.dll"             %INSTALLER_DIR%\plugins\sqldrivers\

if "%ARCH%"=="x86_64" (
    "%ProgramFiles(x86)%\NSIS\makensis.exe" "%BUILD_DIR%\EiskaltDC++.nsi"
) else (
    "%ProgramFiles%\NSIS\makensis.exe" "%BUILD_DIR%\EiskaltDC++.nsi"
)
