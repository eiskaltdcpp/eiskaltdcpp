You can get the lastest program sources from git repository:
download sources on link http://nodeload.github.com/eiskaltdcpp/eiskaltdcpp/tarball/master
or download and install git from http://git-scm.com/
and run in command line: git clone git://github.com/eiskaltdcpp/eiskaltdcpp.git

*******************************************************************************
Build Depends
*******************************************************************************
mingw-get-inst-20120426.exe or later
	http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/mingw-get-inst-20120426/mingw-get-inst-20120426.exe/download

qt-win-opensource-4.8.3-mingw.exe or later
	http://releases.qt-project.org/qt4/source/qt-win-opensource-4.8.3-mingw.exe

Win32OpenSSL-1_0_1e.exe or later
	http://www.slproweb.com/download/Win32OpenSSL-1_0_1e.exe

cmake-2.8.9-win32-x86.exe or later
	http://www.cmake.org/files/v2.8/cmake-2.8.9-win32-x86.exe

nsis-2.46-setup.exe or later
	http://sourceforge.net/projects/nsis/files/NSIS%202/2.46/nsis-2.46-setup.exe/download

boost_1_49_0.tar.bz2 or later
	http://sourceforge.net/projects/boost/files/boost/1.49.0/boost_1_49_0.tar.bz2/download

bzip2-1.0.6-4-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/bzip2/bzip2-1.0.6-4/bzip2-1.0.6-4-mingw32-dev.tar.lzma/download

libbz2-1.0.6-4-mingw32-dll-2.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/bzip2/bzip2-1.0.6-4/libbz2-1.0.6-4-mingw32-dll-2.tar.lzma/download

libz-1.2.7-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/zlib/zlib-1.2.7-1/libz-1.2.7-1-mingw32-dev.tar.lzma/download

libz-1.2.7-1-mingw32-dll-1.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/zlib/zlib-1.2.7-1/libz-1.2.7-1-mingw32-dll-1.tar.lzma/download

gettext-0.18.1.1-2-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/gettext/gettext-0.18.1.1-2/gettext-0.18.1.1-2-mingw32-dev.tar.lzma/download

libgettextpo-0.18.1.1-2-mingw32-dll-0.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/gettext/gettext-0.18.1.1-2/libgettextpo-0.18.1.1-2-mingw32-dll-0.tar.lzma/download

libintl-0.18.1.1-2-mingw32-dll-8.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/gettext/gettext-0.18.1.1-2/libintl-0.18.1.1-2-mingw32-dll-8.tar.lzma/download

libiconv-1.14-2-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/libiconv/libiconv-1.14-2/libiconv-1.14-2-mingw32-dev.tar.lzma/download

libiconv-1.14-2-mingw32-dll-2.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/libiconv/libiconv-1.14-2/libiconv-1.14-2-mingw32-dll-2.tar.lzma/download

mingw32-lua-5.1.4-2.zip or later
	http://sourceforge.net/projects/mingw-cross/files/[LIB] Lua/mingw32-lua-5.1.4-2/mingw32-lua-5.1.4-2.zip/download

aspell-0.60.5-1-bin.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-bin.tar.bz2

aspell-0.60.5-1-lib.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-lib.tar.bz2

libidn-1.25-win32.zip or later
	ftp://ftp.gnu.org/gnu/libidn/libidn-1.25-win32.zip

mingw32-qtscriptgenerator-git-26-10-2010.tar.xz or later
	http://eiskaltdc.googlecode.com/files/mingw32-qtscriptgenerator-git-26-10-2010.tar.xz

mingw32-pcre-8.21.tar.xz or later
	http://eiskaltdc.googlecode.com/files/mingw32-pcre-8.21.tar.xz

*******************************************************************************
Compilation in MS Windows
*******************************************************************************
Install:
	mingw-get-inst-*.exe (need C and C++ compilers)
	qt-win-opensource-*.exe
	Win32OpenSSL-*.exe
	cmake-*.exe
	nsis-*.exe

Make directory mingw32-depends and go into it, create directories there: aspell, boost, gettext, bzip2, iconv, idna, zip, lua, pcre.

Unpack directory boost from boost* archive to mingw32-depends\boost\include\
Unpack bzip2-*, libbz2-* archives to mingw32-depends\bzip2\
Unpack libz-* archives to mingw32-depends\zip\
Unpack gettext-*, libgettextpo-*, libintl-* archives to mingw32-depends\gettext\
Unpack libiconv-* archives to mingw32-depends\iconv\
Unpack mingw32-lua-*/mingw32-lua-*/ archive to mingw32-depends\lua\
Unpack aspell-* archives to mingw32-depends\aspell\
Unpack libidn-* archive to mingw32-depends\idna\
Unpack mingw32-pcre-* archive to mingw32-depends\pcre\
Unpack mingw32-qtscriptgenerator-* archive to mingw32-depends\
--Note: If the libidn-* archive have a file "lib/Libidn.dll" delete it.

Check variables.bat for correct paths and run:
	configure.bat
	build.bat
	install.bat

If you need installer run:
	create_installer.bat

*******************************************************************************
Compilation in Linux ( cross-compile )
*******************************************************************************
Install packages for cross-compile - mingw*
Install package cmake
Install package nsis
Install(in "wine") qt-win-opensource-*.exe and Win32OpenSSL-*.exe

Make directory mingw32-depends and go into it, create directories there: aspell, boost, gettext, bzip2, iconv, idna, zip, lua, pcre.

Unpack directory boost from boost* archive to mingw32-depends/boost/include/
Unpack bzip2-*, libbz2-* archives to mingw32-depends/bzip2/
Unpack libz-* archives to mingw32-depends/zip/
Unpack gettext-*, libgettextpo-*, libintl-* archives to mingw32-depends/gettext/
Unpack libiconv-* archives to mingw32-depends/iconv/
Unpack mingw32-lua-*/mingw32-lua-*/ archive to mingw32-depends/lua/
Unpack aspell-* archives to mingw32-depends/aspell/
Unpack libidn-* archive to mingw32-depends/idna/
Unpack mingw32-pcre-* archive to mingw32-depends/pcre/
Unpack mingw32-qtscriptgenerator-* archive to mingw32-depends/
--Note: If the libidn-* archive have a file "lib/Libidn.dll" delete it.

Check variables.sh for correct paths and run:
        ./configure.sh
        ./build.sh
        ./install.sh

If you need installer run:
        ./create_installer.sh
