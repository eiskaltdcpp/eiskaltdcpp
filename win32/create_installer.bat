call variables.bat
set DIRINSTALLER="%SOURCESDIR%\win32\installer"
IF NOT EXIST "%DIRINSTALLER%" MKDIR %DIRINSTALLER%

strip "%SOURCESDIR%\win32\eiskaltdcpp-qt\EiskaltDC++ Qt.exe"

mingw32-make -k install DESTDIR=%DIRINSTALLER%

copy /Y "%DIRINSTALLER%\Program Files\EiskaltDC++\EiskaltDC++ Qt.exe" %DIRINSTALLER%
move /Y "%DIRINSTALLER%\Program Files\EiskaltDC++\resources"          %DIRINSTALLER%
rmdir /s /q "%DIRINSTALLER%\Program Files"

copy /Y "%SOURCESDIR%\icons\eiskaltdcpp.ico"           %DIRINSTALLER%
copy /Y "%SOURCESDIR%\icons\icon_128x128.bmp"          %DIRINSTALLER%
copy /Y "%SOURCESDIR%\win32\dcppboot.xml"                             %DIRINSTALLER%
copy /Y "%SOURCESDIR%\LICENSE"                                        %DIRINSTALLER%

copy /Y "%QTDIR%\bin\QtCore4.dll"                                     %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtGui4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtNetwork4.dll"                                  %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtXml4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtScript4.dll"                                   %DIRINSTALLER%

copy /Y "%MINGW%\bin\aspell-15.dll"                                   %DIRINSTALLER%

copy /Y "%MINGW%\bin\ssleay32.dll"                                    %DIRINSTALLER%
copy /Y "%MINGW%\bin\libeay32.dll"                                    %DIRINSTALLER%

copy /Y "%MINGW%\bin\mingwm10.dll"                                    %DIRINSTALLER%
copy /Y "%MINGW%\bin\intl.dll"                                        %DIRINSTALLER%
copy /Y "%MINGW%\bin\iconv.dll"                                       %DIRINSTALLER%
copy /Y "%MINGW%\bin\mgwz.dll"                                        %DIRINSTALLER%
copy /Y "%MINGW%\bin\mgwbz2-1.dll"                                    %DIRINSTALLER%
copy /Y "%MINGW%\bin\libgcc_s_dw2-1.dll"                              %DIRINSTALLER%

"%ProgramFiles%\NSIS\makensis.exe" "%SOURCESDIR%\win32\EiskaltDC++.nsi"
