set QTSDKDIR=C:\Qt\2010.01


set QTDIR=%QTSDKDIR%\qt
set PATH=%QTDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++


mkdir EiskaltDC++
copy /Y "%QTDIR%\bin\mingwm10.dll" .\EiskaltDC++\
copy /Y "%QTDIR%\bin\QtCore4.dll" .\EiskaltDC++\
copy /Y "%QTDIR%\bin\QtNetwork4.dll" .\EiskaltDC++\
copy /Y "%QTDIR%\bin\QtSql4.dll" .\EiskaltDC++\

