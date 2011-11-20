set CMAKE_DIR=%ProgramFiles%\CMake 2.8
set INSTALL_DIR=%ProgramFiles%\EiskaltDC++
set SOURCES_DIR=%SystemDrive%\eiskaltdcpp
set BUILD_DIR=.
set INSTALLER_DIR=%BUILD_DIR%\installer

set MINGW32_DIR=%SystemDrive%\MinGW
set QT_MINGW32_DIR=%SystemDrive%\Qt\4.7.4

set MINGW32_DEPENDS_DIR=%SystemDrive%\mingw32-depends
set ASPELL_DIR=%MINGW32_DEPENDS_DIR%\aspell
set BOOST_DIR=%MINGW32_DEPENDS_DIR%\boost
set BOOST_HEADERS_DIR=%BOOST_DIR%\include
set GETTEXT_DIR=%MINGW32_DEPENDS_DIR%\gettext
set GETTEXT_HEADERS_DIR=%GETTEXT_DIR%\include
set LIBBZ2_DIR=%MINGW32_DEPENDS_DIR%\libbz2
set LIBICONV_DIR=%MINGW32_DEPENDS_DIR%\libiconv
set LIBIDN_DIR=%MINGW32_DEPENDS_DIR%\libidn
set LIBZ_DIR=%MINGW32_DEPENDS_DIR%\libz
set LUA_DIR=%MINGW32_DEPENDS_DIR%\lua

set PATH=%QT_MINGW32_DIR%\bin
set PATH=%PATH%;%MINGW32_DIR%\bin
set PATH=%PATH%;%CMAKE_DIR%\bin
set PATH=%PATH%;%SystemRoot%\System32
