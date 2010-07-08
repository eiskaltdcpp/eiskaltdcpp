set QTSDKDIR=C:\Qt\2010.01


set QTDIR=%QTSDKDIR%\qt
set PATH=%QTDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++


mingw32-make -j2

