set QTSDKDIR=%SystemDrive%\Qt\2010.01
set CMAKEDIR="%ProgramFiles%\CMake 2.8"
set TARGET="%ProgramFiles%\EiskaltDC++"

@REM For example:
@REM set QTSDKDIR="C:\Qt\2010.01"
@REM set CMAKEDIR="C:\Program Files\CMake 2.8"
@REM set TARGET="C:\Program Files\EiskaltDC++"

set MINGW=%QTSDKDIR%\mingw
set QTDIR=%QTSDKDIR%\qt

set PATH=%QTDIR%\bin
set PATH=%PATH%;%MINGW%\bin
set PATH=%PATH%;%CMAKEDIR%\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

