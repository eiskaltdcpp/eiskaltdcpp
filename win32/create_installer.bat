call variables.bat

set DIRINSTALLER=%BUILDDIR%\installer

mingw32-make -k install DESTDIR=%BUILDDIR%

xcopy /E /R /Y /I "%BUILDDIR%\Program Files\EiskaltDC++\*"            %DIRINSTALLER%
rmdir /s /q "%BUILDDIR%\Program Files"

strip "%DIRINSTALLER%\eiskaltdcpp-qt.exe"
strip "%DIRINSTALLER%\eiskaltdcpp-daemon.exe"

copy /Y "%SOURCESDIR%\icons\eiskaltdcpp.ico"                          %DIRINSTALLER%
copy /Y "%SOURCESDIR%\icons\icon_164x314.bmp"                         %DIRINSTALLER%
copy /Y "%SOURCESDIR%\win32\dcppboot.xml"                             %DIRINSTALLER%
copy /Y "%SOURCESDIR%\LICENSE"                                        %DIRINSTALLER%
echo [Paths] > "%DIRINSTALLER%\qt.conf"
echo Plugins = ./plugins >> "%DIRINSTALLER%\qt.conf"

copy /Y "%QTDIR%\bin\QtCore4.dll"                                     %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtGui4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtNetwork4.dll"                                  %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtXml4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtScript4.dll"                                   %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtDeclarative4.dll"                              %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtSql4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtXmlPatterns4.dll"                              %DIRINSTALLER%

mkdir "%DIRINSTALLER%\script\"
copy /Y "%MINGW%\bin\qtscript_core.dll"                               %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_gui.dll"                                %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_network.dll"                            %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_opengl.dll"                             %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_phonon.dll"                             %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_sql.dll"                                %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_svg.dll"                                %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_uitools.dll"                            %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_webkit.dll"                             %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_xml.dll"                                %DIRINSTALLER%\script\
copy /Y "%MINGW%\bin\qtscript_xmlpatterns.dll"                        %DIRINSTALLER%\script\

copy /Y "%MINGW%\bin\libgcc_s_dw2-1.dll"                              %DIRINSTALLER%
copy /Y "%MINGW%\bin\mingwm10.dll"                                    %DIRINSTALLER%

copy /Y "%MINGW%\bin\libintl-8.dll"                                   %DIRINSTALLER%
copy /Y "%MINGW%\bin\libiconv-2.dll"                                  %DIRINSTALLER%
copy /Y "%MINGW%\bin\libz-1.dll"                                      %DIRINSTALLER%
copy /Y "%MINGW%\bin\libbz2-2.dll"                                    %DIRINSTALLER%
copy /Y "%MINGW%\bin\lua51.dll"                                       %DIRINSTALLER%
copy /Y "%MINGW%\bin\libidn-11.dll"                                   %DIRINSTALLER%
copy /Y "%MINGW%\bin\libaspell-15.dll"                                %DIRINSTALLER%

mkdir "%DIRINSTALLER%\aspell\data\"
mkdir "%DIRINSTALLER%\aspell\dict\"
copy /Y "%MINGW%\lib\aspell-0.60\*"                                   %DIRINSTALLER%\aspell\data\

copy /Y "%SystemRoot%\System32\ssleay32.dll"                          %DIRINSTALLER%
copy /Y "%SystemRoot%\System32\libeay32.dll"                          %DIRINSTALLER%

mkdir "%DIRINSTALLER%\plugins\sqldrivers\"
copy /Y "%QTDIR%\plugins\sqldrivers\qsqlite4.dll"                     %DIRINSTALLER%\plugins\sqldrivers\

"%ProgramFiles%\NSIS\makensis.exe" "%BUILDDIR%\EiskaltDC++.nsi"
