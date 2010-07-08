set QTSDKDIR=C:\Qt\2010.01


set QTDIR=%QTSDKDIR%\qt
set PATH=%QTDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++


set TARGET="C:\Program Files\eiskaltdcpp"


mingw32-make -k install
strip "%TARGET%\bin\eiskaltdcpp.exe"


copy /Y "%QTDIR%\bin\mingwm10.dll" "%TARGET%\bin"
copy /Y "%QTDIR%\bin\QtCore4.dll" "%TARGET%\bin"
copy /Y "%QTDIR%\bin\QtGui4.dll" "%TARGET%\bin"
copy /Y "%QTDIR%\bin\QtNetwork4.dll" "%TARGET%\bin"
copy /Y "%QTDIR%\bin\QtXml4.dll" "%TARGET%\bin"


