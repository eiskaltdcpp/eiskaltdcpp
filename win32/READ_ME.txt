*******************************************************************************
Build Depends
*******************************************************************************
  * qt-sdk-win-opensource-2010.01.exe or later

http://get.qt.nokia.com/qtsdk/qt-sdk-win-opensource-2010.01.exe
ftp://ftp.qt.nokia.com/qtsdk/qt-sdk-win-opensource-2010.01.exe

  * Win32OpenSSL-1_0_0a.exe or later

http://www.slproweb.com/download/Win32OpenSSL-1_0_0a.exe
http://www.shininglightpro.com/download/Win32OpenSSL-1_0_0a.exe

  * cmake-2.8.2-win32-x86.exe or later

http://www.cmake.org/cmake/resources/software.html
http://www.cmake.org/files/v2.8/cmake-2.8.2-win32-x86.exe

  * boost.tar.gz

http://code.google.com/p/eiskaltdc/downloads/detail?name=boost.tar.gz
http://eiskaltdc.googlecode.com/files/boost.tar.gz

  * mingw-libbz2-devel-1.0.5-10.tar.bz2

http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-bzip2/mingw-libbz2-devel/mingw-libbz2-devel-1.0.5-10.tar.bz2
http://ftp.daum.net/cygwin/release/mingw/mingw-bzip2/mingw-libbz2-devel/mingw-libbz2-devel-1.0.5-10.tar.bz2

  * mingw-zlib-devel-1.2.3-10.tar.bz2

http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-zlib/mingw-zlib-devel/mingw-zlib-devel-1.2.3-10.tar.bz2
http://ftp.uni-kl.de/pub/windows/cygwin/release/mingw/mingw-zlib/mingw-zlib-devel/mingw-zlib-devel-1.2.3-10.tar.bz2

  * mingw-libbz2_1-1.0.5-10.tar.bz2 or later

http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-bzip2/mingw-libbz2_1/mingw-libbz2_1-1.0.5-10.tar.bz2
http://ftp.daum.net/cygwin/release/mingw/mingw-bzip2/mingw-libbz2_1/mingw-libbz2_1-1.0.5-10.tar.bz2

  * mingw-zlib0-1.2.3-10.tar.bz2 or later

http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-zlib/mingw-zlib0/mingw-zlib0-1.2.3-10.tar.bz2
http://ftp.uni-kl.de/pub/windows/cygwin/release/mingw/mingw-zlib/mingw-zlib0/mingw-zlib0-1.2.3-10.tar.bz2

  * gettext-tools-0.13.1.bin.woe32.zip or later

http://ftp.gnu.org/gnu/gettext/gettext-tools-0.13.1.bin.woe32.zip

  * gettext-runtime-0.13.1.bin.woe32.zip or later

http://ftp.gnu.org/gnu/gettext/gettext-runtime-0.13.1.bin.woe32.zip

  * libiconv-1.9.1.bin.woe32.zip or later

http://ftp.gnu.org/gnu/libiconv/libiconv-1.9.1.bin.woe32.zip



*******************************************************************************
Compilation in MS Windows
*******************************************************************************
  * Install:

qt-sdk-win-opensource-*
Win32OpenSSL-*
cmake-*

  * Unpack boost-* archive to the root directory of eiskaltdcpp sources.

  * Unpack files from mingw-* archives to %QTSDKDIR%\mingw\

  * Unpack files from gettext-* and libiconv-* archives to %QTSDKDIR%\mingw\

  * Edit the files and run them:
configure.bat
make_.bat
make_install.bat

  * Program with all necessary files is now available in EiskaltDC++ directory.


---------
# Note: you can get the lastest program sorces from subversion
svn export http://eiskaltdc.googlecode.com/svn/branches/trunk/ eiskaltdcpp-trunk
You can use Slik-Subversion for this scope.

