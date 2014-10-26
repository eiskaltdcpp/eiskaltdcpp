You can get the lastest program sources from git repository:
download sources on link https://codeload.github.com/eiskaltdcpp/eiskaltdcpp/tar.gz/master
or download and install git from http://git-scm.com/
and run in command line: git clone git://github.com/eiskaltdcpp/eiskaltdcpp.git

*******************************************************************************
Build Depends
*******************************************************************************
i686-4.9.1-release-posix-dwarf-rt_v3-rev1.7z or later
	http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.9.1/threads-posix/dwarf/i686-4.9.1-release-posix-dwarf-rt_v3-rev1.7z/download

qt-opensource-windows-x86-mingw482-4.8.6-1.exe or later
	http://download.qt-project.org/official_releases/qt/4.8/4.8.6/qt-opensource-windows-x86-mingw482-4.8.6-1.exe

Win32OpenSSL-1_0_1j.exe or later
	http://slproweb.com/download/Win32OpenSSL-1_0_1j.exe

cmake-3.0.2-win32-x86.exe or later
	http://www.cmake.org/files/v3.0/cmake-3.0.2-win32-x86.exe

nsis-2.46-setup.exe or later
	http://sourceforge.net/projects/nsis/files/NSIS%202/2.46/nsis-2.46-setup.exe/download

boost_1_49_0.tar.bz2 (for boost versions >= 1.50 required manual build some boost libraries)
	http://sourceforge.net/projects/boost/files/boost/1.49.0/boost_1_49_0.tar.bz2/download

bzip2-1.0.6-4-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/bzip2/bzip2-1.0.6-4/bzip2-1.0.6-4-mingw32-dev.tar.lzma/download

libbz2-1.0.6-4-mingw32-dll-2.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/bzip2/bzip2-1.0.6-4/libbz2-1.0.6-4-mingw32-dll-2.tar.lzma/download

libz-1.2.7-1-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/zlib/zlib-1.2.7-1/libz-1.2.7-1-mingw32-dev.tar.lzma/download

libz-1.2.7-1-mingw32-dll-1.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Extension/zlib/zlib-1.2.7-1/libz-1.2.7-1-mingw32-dll-1.tar.lzma/download

gettext-0.18.3.2-1-mingw32-dev.tar.xz or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/gettext/gettext-0.18.3.2-1/gettext-0.18.3.2-1-mingw32-dev.tar.xz/download

libgettextpo-0.18.3.2-1-mingw32-dll-0.tar.xz or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/gettext/gettext-0.18.3.2-1/libgettextpo-0.18.3.2-1-mingw32-dll-0.tar.xz/download

libintl-0.18.3.2-1-mingw32-dll-8.tar.xz or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/gettext/gettext-0.18.3.2-1/libintl-0.18.3.2-1-mingw32-dll-8.tar.xz/download

libiconv-1.14-3-mingw32-dev.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/libiconv/libiconv-1.14-3/libiconv-1.14-3-mingw32-dev.tar.lzma/download

libiconv-1.14-3-mingw32-dll.tar.lzma or later
	http://sourceforge.net/projects/mingw/files/MinGW/Base/libiconv/libiconv-1.14-3/libiconv-1.14-3-mingw32-dll.tar.lzma/download

mingw32-lua-5.1.4-2.zip or later
	http://sourceforge.net/projects/mingw-cross/files/[LIB] Lua/mingw32-lua-5.1.4-2/mingw32-lua-5.1.4-2.zip/download

aspell-0.60.5-1-bin.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-bin.tar.bz2

aspell-0.60.5-1-lib.tar.bz2 or later
	http://www.winkde.org/pub/kde/ports/win32/repository/aspell/aspell-0.60.5-1-lib.tar.bz2

libidn-1.29-win32.zip or later
	ftp://ftp.gnu.org/gnu/libidn/libidn-1.29-win32.zip

mingw32-qtscriptgenerator-git-26-10-2010.tar.xz or later
	http://eiskaltdc.googlecode.com/files/mingw32-qtscriptgenerator-git-26-10-2010.tar.xz

mingw32-pcre-8.21.tar.xz or later
	http://eiskaltdc.googlecode.com/files/mingw32-pcre-8.21.tar.xz

ShellExecAsUser.zip
	http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip
*******************************************************************************
Compilation in MS Windows
*******************************************************************************
Install:
	qt-opensource-windows-x86-*.exe
	Win32OpenSSL-*.exe
	cmake-*.exe
	nsis-*.exe

Unpack i686-*-release-posix-dwarf-*.7z archive to C:\

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
Unpack ShellExecAsUser.dll from ShellExecAsUser.zip archive to C:\Program Files\NSIS\Plugins\ (for 64-bit to C:\Program Files (x86)\NSIS\Plugins\)
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
Install packages for cross-compile - mingw-w64*
Install package cmake
Install package nsis
Install (in "wine") qt-opensource-windows-x86-*.exe and Win32OpenSSL-*.exe

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
Unpack ShellExecAsUser.dll from ShellExecAsUser.zip archive to /usr/share/nsis/Plugins/
--Note: If the libidn-* archive have a file "lib/Libidn.dll" delete it.

Check variables.sh for correct paths and run:
        ./configure.sh
        ./build.sh
        ./install.sh

If you need installer run:
        ./create_installer.sh
