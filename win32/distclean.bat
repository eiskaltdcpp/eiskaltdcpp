call variables.bat

rmdir /s/q %BUILDDIR%\CMakeFiles
rmdir /s/q %BUILDDIR%\cmake
rmdir /s/q %BUILDDIR%\data
rmdir /s/q %BUILDDIR%\dcpp
rmdir /s/q %BUILDDIR%\dht
rmdir /s/q %BUILDDIR%\eiskaltdcpp-daemon
rmdir /s/q %BUILDDIR%\eiskaltdcpp-qt
rmdir /s/q %BUILDDIR%\extra
rmdir /s/q %BUILDDIR%\json
rmdir /s/q %BUILDDIR%\upnp

del /f %BUILDDIR%\CMakeCache.txt
del /f %BUILDDIR%\CPackConfig.cmake
del /f %BUILDDIR%\CPackSourceConfig.cmake
del /f %BUILDDIR%\Makefile
del /f %BUILDDIR%\VersionGlobal.h
del /f %BUILDDIR%\cmake_install.cmake
del /f %BUILDDIR%\cmake_uninstall.cmake
