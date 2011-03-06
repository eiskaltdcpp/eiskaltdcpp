call variables.bat

mingw32-make -k install

strip "%INSTALLDIR%\EiskaltDC++ Qt.exe"

copy /Y %SOURCESDIR%\win32\dcppboot.xml      %INSTALLDIR%

copy /Y "%QTDIR%\bin\QtCore4.dll"            %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtGui4.dll"             %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtNetwork4.dll"         %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtXml4.dll"             %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtScript4.dll"          %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtDeclarative4.dll"     %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtSql4.dll"             %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtXmlPatterns4.dll"     %INSTALLDIR%

copy /Y "%MINGW%\bin\libgcc_s_dw2-1.dll"     %INSTALLDIR%
copy /Y "%MINGW%\bin\mingwm10.dll"           %INSTALLDIR%

copy /Y "%MINGW%\bin\libintl-8.dll"          %INSTALLDIR%
copy /Y "%MINGW%\bin\iconv.dll"              %INSTALLDIR%
copy /Y "%MINGW%\bin\mgwz.dll"               %INSTALLDIR%
copy /Y "%MINGW%\bin\mgwbz2-1.dll"           %INSTALLDIR%
copy /Y "%MINGW%\bin\libaspell-15.dll"       %INSTALLDIR%
copy /Y "%MINGW%\bin\lua51.dll"              %INSTALLDIR%

copy /Y "%SystemRoot%\System32\ssleay32.dll" %INSTALLDIR%
copy /Y "%SystemRoot%\System32\libeay32.dll" %INSTALLDIR%
