set CMAKE_DIR=%ProgramFiles%\CMake 2.8
set INSTALL_DIR=%ProgramFiles%\EiskaltDC++
set SOURCES_DIR=%SystemDrive%\eiskaltdcpp\eiskaltdcpp
set BUILD_DIR=.
set INSTALLER_DIR=%BUILD_DIR%\installer

set MINGW32_DIR=%SystemDrive%\MinGW
set QT_MINGW32_DIR=%SystemDrive%\Qt\4.8.0

set MINGW32_DEPENDS_DIR=%SystemDrive%\eiskaltdcpp\mingw32-depends
set ASPELL_DIR=%MINGW32_DEPENDS_DIR%\aspell
set OPENSSL_DIR=%MINGW32_DEPENDS_DIR%\OpenSSL-Win32
set BOOST_DIR=%MINGW32_DEPENDS_DIR%\boost
set GETTEXT_DIR=%MINGW32_DEPENDS_DIR%\gettext
set LIBBZ2_DIR=%MINGW32_DEPENDS_DIR%\bzip2
set LIBICONV_DIR=%MINGW32_DEPENDS_DIR%\iconv
set LIBIDN_DIR=%MINGW32_DEPENDS_DIR%\idna
set LIBZ_DIR=%MINGW32_DEPENDS_DIR%\zip
set LUA_DIR=%MINGW32_DEPENDS_DIR%\lua
set PCRE_DIR=%MINGW32_DEPENDS_DIR%\pcre

@rem Note: since paths to headers will be used in the GCC flags it must use slash "/" instead backslash "\".
set BOOST_HEADERS_DIR=C:/eiskaltdcpp/mingw32-depends/boost/include
set GETTEXT_HEADERS_DIR=C:/eiskaltdcpp/mingw32-depends/gettext/include
set PCRE_HEADERS_DIR=C:/eiskaltdcpp/mingw32-depends/pcre/include

set PATH=%QT_MINGW32_DIR%\bin
set PATH=%PATH%;%MINGW32_DIR%\bin
set PATH=%PATH%;%CMAKE_DIR%\bin
set PATH=%PATH%;%SystemRoot%\System32
