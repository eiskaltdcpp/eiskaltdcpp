call variables.bat

mingw32-make -k install

strip %TARGET%\EiskaltDC++.exe


copy /Y "%QTDIR%\bin\QtCore4.dll"  %TARGET%
copy /Y "%QTDIR%\bin\QtGui4.dll"   %TARGET%
copy /Y "%QTDIR%\bin\QtNetwork4.dll" %TARGET%
copy /Y "%QTDIR%\bin\QtXml4.dll" %TARGET%

copy /Y "%MINGW%\bin\mingwm10.dll" %TARGET%
copy /Y "%MINGW%\bin\intl.dll"     %TARGET%
copy /Y "%MINGW%\bin\iconv.dll"    %TARGET%
copy /Y "%MINGW%\bin\mgwz.dll"     %TARGET%
copy /Y "%MINGW%\bin\mgwbz2-1.dll" %TARGET%
copy /Y "%MINGW%\bin\libgcc_s_dw2-1.dll" %TARGET%
