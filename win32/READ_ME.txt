You can get the lastest program sorces from git repository:
download sources on link http://nodeload.github.com/negativ/eiskaltdcpp/tarball/master
or download and install git from http://git-scm.com/
and run in command line: git clone git://github.com/negativ/eiskaltdcpp.git

*******************************************************************************
Build Depends
*******************************************************************************
qt-sdk-win-opensource-2010.05.exe or later
	http://get.qt.nokia.com/qtsdk/qt-sdk-win-opensource-2010.05.exe
	ftp://ftp.qt.nokia.com/qtsdk/qt-sdk-win-opensource-2010.05.exe

Win32OpenSSL-1_0_0d.exe or later
	http://www.slproweb.com/download/Win32OpenSSL-1_0_0d.exe
	http://www.shininglightpro.com/download/Win32OpenSSL-1_0_0d.exe

cmake-2.8.4-win32-x86.exe or later
	http://www.cmake.org/files/v2.8/cmake-2.8.4-win32-x86.exe

nsis-2.46-setup.exe or later
	http://sourceforge.net/projects/nsis/files/NSIS%202/2.46/nsis-2.46-setup.exe/download

boost_1_46_0.tar.bz2 or later
	http://sourceforge.net/projects/boost/files/boost/1.46.0/boost_1_46_0.tar.bz2/download

mingw-libbz2-devel-1.0.5-10.tar.bz2
	http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-bzip2/mingw-libbz2-devel/mingw-libbz2-devel-1.0.5-10.tar.bz2
	http://ftp.daum.net/cygwin/release/mingw/mingw-bzip2/mingw-libbz2-devel/mingw-libbz2-devel-1.0.5-10.tar.bz2

mingw-libbz2_1-1.0.5-10.tar.bz2 or later
	http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-bzip2/mingw-libbz2_1/mingw-libbz2_1-1.0.5-10.tar.bz2
	http://ftp.daum.net/cygwin/release/mingw/mingw-bzip2/mingw-libbz2_1/mingw-libbz2_1-1.0.5-10.tar.bz2

mingw-zlib-devel-1.2.3-10.tar.bz2
	http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-zlib/mingw-zlib-devel/mingw-zlib-devel-1.2.3-10.tar.bz2
	http://ftp.uni-kl.de/pub/windows/cygwin/release/mingw/mingw-zlib/mingw-zlib-devel/mingw-zlib-devel-1.2.3-10.tar.bz2

mingw-zlib0-1.2.3-10.tar.bz2 or later
	http://ring.nict.go.jp/archives/pc/gnu-win32/release/mingw/mingw-zlib/mingw-zlib0/mingw-zlib0-1.2.3-10.tar.bz2
	http://ftp.uni-kl.de/pub/windows/cygwin/release/mingw/mingw-zlib/mingw-zlib0/mingw-zlib0-1.2.3-10.tar.bz2

gettext-0.17-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/gettext/gettext-0.17-1/gettext-0.17-1-mingw32-dev.tar.lzma/download

libgettextpo-0.17-1-mingw32-dll-0.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/gettext/gettext-0.17-1/libgettextpo-0.17-1-mingw32-dll-0.tar.lzma/download

libintl-0.17-1-mingw32-dll-8.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/gettext/gettext-0.17-1/libintl-0.17-1-mingw32-dll-8.tar.lzma/download

libiconv-1.9.1.bin.woe32.zip or later
	http://ftp.gnu.org/gnu/libiconv/libiconv-1.9.1.bin.woe32.zip

mingw32-lua-5.1.4-2.zip or later
	http://sourceforge.net/projects/mingw-cross/files/[LIB] Lua/mingw32-lua-5.1.4-2/mingw32-lua-5.1.4-2.zip/download

aspell-0.60.5-1-bin.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-bin.tar.bz2

aspell-0.60.5-1-lib.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-lib.tar.bz2

libidn-1.11-w32.zip or later
	http://www.gknw.de/mirror/libidn/win32/libidn-1.11-w32.zip

mingw32-qtscriptgenerator-git-26-10-2010.tar.xz or later
	http://eiskaltdc.googlecode.com/files/mingw32-qtscriptgenerator-git-26-10-2010.tar.xz

*******************************************************************************
Compilation in MS Windows
*******************************************************************************
Install:
	qt-sdk-win-opensource-*
	Win32OpenSSL-*
	cmake-*
	nsis-*
Unpack directory boost from boost* archive to %QTSDKDIR%\mingw\include\.
Unpack files from mingw-* and mingw32-qtscriptgenerator-* archives to appropriate subdirectories in %QTSDKDIR%\mingw\
( i.e. bzlib.h shoud go to %QTSDKDIR%\mingw\include\
       libbz2.a         to %QTSDKDIR%\mingw\lib\
       mgwbz2-1.dll     to %QTSDKDIR%\mingw\bin\ )
Unpack files from gettext-*, libgettextpo-*, libintl-*, libidn-*, aspell-* and libiconv-* archives to %QTSDKDIR%\mingw\
Check variables.bat for correct paths and run:
	configure.bat
	build.bat
	install.bat
Program with all necessary files is now available in EiskaltDC++ directory.
If you need installer run create_installer.bat

*******************************************************************************
Compilation in Linux ( cross-compile )
*******************************************************************************
Install packages for cross-compile - mingw*
Install package cmake
Install(in "wine") qt-sdk-win-opensource-*.exe and Win32OpenSSL-*.exe
Make directory "win32-depends" and unpack dependencies(above) with this structure directories:
	--> win32-depends
		--> bin (*.dll)
		--> include(*.h or subdirectory with headers, example "openssl")
		--> lib (*.dll.a *.a)
Edit file Toolchain-mingw32.cmake, correct these variables in accordance with your system:
MINGW32_NAME
QT_WIN32_PREFIX
add in CMAKE_FIND_ROOT_PATH path to directory win32-depends and path to openssl directory from wine installation
Check cmake flags in file configure.sh and path to eiskaltdcpp sources directory
Build:
	./configure.sh
	make
	make install

--Note
gcc finds the header files only if they are in the system directory, example "/usr/i486-mingw32"

Install package nsis
Make directory "installer" and copy there files and directories listed in file EiskaltDC++.nsi (Section "EiskaltDC++")
Create installer:
	makensis EiskaltDC++.nsi
