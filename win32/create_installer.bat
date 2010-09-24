call variables.bat

set DIRINSTALLER="%SOURCESDIR%\win32\installer"

mingw32-make -k install DESTDIR=%BUILDDIR%

xcopy /E /R /Y /I "%BUILDDIR%\Program Files\EiskaltDC++\*"            %DIRINSTALLER%
rmdir /s /q "%BUILDDIR%\Program Files"

strip "%DIRINSTALLER%\EiskaltDC++ Qt.exe"

copy /Y "%SOURCESDIR%\icons\eiskaltdcpp.ico"                          %DIRINSTALLER%
copy /Y "%SOURCESDIR%\icons\icon_164x314.bmp"                         %DIRINSTALLER%
copy /Y "%SOURCESDIR%\win32\dcppboot.xml"                             %DIRINSTALLER%
copy /Y "%SOURCESDIR%\LICENSE"                                        %DIRINSTALLER%

copy /Y "%QTDIR%\bin\QtCore4.dll"                                     %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtGui4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtNetwork4.dll"                                  %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtXml4.dll"                                      %DIRINSTALLER%
copy /Y "%QTDIR%\bin\QtScript4.dll"                                   %DIRINSTALLER%

copy /Y "%MINGW%\bin\libgcc_s_dw2-1.dll"                              %DIRINSTALLER%
copy /Y "%MINGW%\bin\mingwm10.dll"                                    %DIRINSTALLER%

copy /Y "%MINGW%\bin\intl.dll"                                        %DIRINSTALLER%
copy /Y "%MINGW%\bin\iconv.dll"                                       %DIRINSTALLER%
copy /Y "%MINGW%\bin\mgwz.dll"                                        %DIRINSTALLER%
copy /Y "%MINGW%\bin\mgwbz2-1.dll"                                    %DIRINSTALLER%
copy /Y "%MINGW%\bin\lua51.dll"                                       %DIRINSTALLER%
@rem copy /Y "%MINGW%\bin\aspell-15.dll"                                   %DIRINSTALLER%

copy /Y "%SystemRoot%\System32\ssleay32.dll"                          %DIRINSTALLER%
copy /Y "%SystemRoot%\System32\libeay32.dll"                          %DIRINSTALLER%

"%ProgramFiles%\NSIS\makensis.exe" "%SOURCESDIR%\win32\EiskaltDC++.nsi"
