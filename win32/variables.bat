@REM set QTSDKDIR=%SystemDrive%\Qt\2010.05
set QTSDKDIR=%SystemDrive%\QtSDK\Desktop\Qt\4.7.4\mingw
set CMAKEDIR=%ProgramFiles%\CMake 2.8
set INSTALLDIR=%ProgramFiles%\EiskaltDC++
set SOURCESDIR=..
set BUILDDIR=.

@REM For example:
@REM set QTSDKDIR=C:\Qt\2010.05
@REM set CMAKEDIR=C:\Program Files\CMake 2.8
@REM set INSTALLDIR=C:\Program Files\EiskaltDC++

set MINGW=%QTSDKDIR%\mingw
set QTDIR=%QTSDKDIR%\qt

set PATH=%QTDIR%\bin
set PATH=%PATH%;%MINGW%\bin
set PATH=%PATH%;%CMAKEDIR%\bin
set PATH=%PATH%;%SystemRoot%\System32
