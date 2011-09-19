You can get the lastest program sources from git repository:
download sources on link http://nodeload.github.com/negativ/eiskaltdcpp/tarball/master
or download and install git from http://git-scm.com/
and run in command line: git clone git://github.com/negativ/eiskaltdcpp.git

*******************************************************************************
Build Depends
*******************************************************************************
Qt_SDK_Win_offline_v1_1_3_en.exe or Qt_SDK_Win_online_v1_1_3_en.exe or later
	http://get.qt.nokia.com/qtsdk/Qt_SDK_Win_offline_v1_1_3_en.exe (offline installer)
	ftp://ftp.qt.nokia.com/qtsdk/Qt_SDK_Win_offline_v1_1_3_en.exe  (offline installer)
	http://get.qt.nokia.com/qtsdk/Qt_SDK_Win_online_v1_1_3_en.exe  (online installer)
	ftp://ftp.qt.nokia.com/qtsdk/Qt_SDK_Win_online_v1_1_3_en.exe   (online installer)

Win32OpenSSL-1_0_0e.exe or later
	http://www.slproweb.com/download/Win32OpenSSL-1_0_0e.exe
	http://www.shininglightpro.com/download/Win32OpenSSL-1_0_0e.exe

cmake-2.8.5-win32-x86.exe or later
	http://www.cmake.org/files/v2.8/cmake-2.8.5-win32-x86.exe

nsis-2.46-setup.exe or later
	http://sourceforge.net/projects/nsis/files/NSIS%202/2.46/nsis-2.46-setup.exe/download

boost_1_47_0.tar.bz2 or later
	http://sourceforge.net/projects/boost/files/boost/1.47.0/boost_1_47_0.tar.bz2/download

bzip2-1.0.6-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/bzip2/1.0.6-1/bzip2-1.0.6-1-mingw32-dev.tar.lzma/download

libbz2-1.0.6-1-mingw32-dll-2.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/bzip2/1.0.6-1/libbz2-1.0.6-1-mingw32-dll-2.tar.lzma/download

libz-1.2.5-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/zlib/zlib-1.2.5-1/libz-1.2.5-1-mingw32-dev.tar.lzma/download

libz-1.2.5-1-mingw32-dll-1.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/zlib/zlib-1.2.5-1/libz-1.2.5-1-mingw32-dll-1.tar.lzma/download

gettext-0.17-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/gettext/gettext-0.17-1/gettext-0.17-1-mingw32-dev.tar.lzma/download

libgettextpo-0.17-1-mingw32-dll-0.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/gettext/gettext-0.17-1/libgettextpo-0.17-1-mingw32-dll-0.tar.lzma/download

libintl-0.17-1-mingw32-dll-8.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/gettext/gettext-0.17-1/libintl-0.17-1-mingw32-dll-8.tar.lzma/download

libiconv-1.13.1-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/libiconv/libiconv-1.13.1-1/libiconv-1.13.1-1-mingw32-dev.tar.lzma/download

libiconv-1.13.1-1-mingw32-dll-2.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/libiconv/libiconv-1.13.1-1/libiconv-1.13.1-1-mingw32-dll-2.tar.lzma/download

mingw32-lua-5.1.4-2.zip or later
	http://sourceforge.net/projects/mingw-cross/files/[LIB] Lua/mingw32-lua-5.1.4-2/mingw32-lua-5.1.4-2.zip/download

aspell-0.60.5-1-bin.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-bin.tar.bz2

aspell-0.60.5-1-lib.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-lib.tar.bz2

libidn-1.22-win32.zip or later
	ftp://ftp.gnu.org/gnu/libidn/libidn-1.22-win32.zip

mingw32-qtscriptgenerator-git-26-10-2010.tar.xz or later
	http://eiskaltdc.googlecode.com/files/mingw32-qtscriptgenerator-git-26-10-2010.tar.xz

*******************************************************************************
Compilation in MS Windows
*******************************************************************************
Install:
	Qt_SDK_Win_*.exe
	Win32OpenSSL-*.exe
	cmake-*.exe
	nsis-*.exe
Unpack directory boost from boost* archive to %QTSDKDIR%\mingw\include\.
Unpack files from mingw32-lua-* and mingw32-qtscriptgenerator-* archives to appropriate subdirectories in %QTSDKDIR%\mingw\
( i.e. *.h should go to %QTSDKDIR%\mingw\include\
       *.dll.a *.a   to %QTSDKDIR%\mingw\lib\
       *.dll         to %QTSDKDIR%\mingw\bin\ )
Unpack files from bzip2-*, libbz2-*, libz-*, gettext-*, libgettextpo-*, libintl-*, libiconv-*, libidn-* and aspell-* archives to %QTSDKDIR%\mingw\
Check variables.bat for correct paths and run:
	configure.bat
	build.bat
	install.bat
Program with all necessary files is now available in EiskaltDC++ directory.
If you need installer run:
	create_installer.bat

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
