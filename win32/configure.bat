set QTSDKDIR=C:\Qt\2010.01


set QTDIR=%QTSDKDIR%\qt
set PATH=%QTDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\bin
set PATH=%PATH%;%QTSDKDIR%\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++


cmake -DUSE_ASPELL:BOOL=0 -DUSE_LIBUPNP=OFF -DFORCE_XDG=OFF -DFREE_SPACE_BAR_C:BOOL=1 ./

