call variables.bat

mingw32-make -k install

mkdir %INSTALLDIR%

xcopy /E/C/R/Y %BUILDDIR%\resources  %INSTALLDIR%\resources\

copy /Y "%BUILDDIR%\eiskaltdcpp\EiskaltDC++.exe"  %INSTALLDIR%
strip %INSTALLDIR%\EiskaltDC++.exe

copy /Y %BUILDDIR%\dcppboot.xml  %INSTALLDIR%

copy /Y "%QTDIR%\bin\QtCore4.dll"  %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtGui4.dll"   %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtNetwork4.dll" %INSTALLDIR%
copy /Y "%QTDIR%\bin\QtXml4.dll" %INSTALLDIR%

copy /Y "%MINGW%\bin\mingwm10.dll" %INSTALLDIR%
copy /Y "%MINGW%\bin\intl.dll"     %INSTALLDIR%
copy /Y "%MINGW%\bin\iconv.dll"    %INSTALLDIR%
copy /Y "%MINGW%\bin\mgwz.dll"     %INSTALLDIR%
copy /Y "%MINGW%\bin\mgwbz2-1.dll" %INSTALLDIR%
copy /Y "%MINGW%\bin\libgcc_s_dw2-1.dll" %INSTALLDIR%
