call variables.bat

rmdir /s/q %BUILDDIR%\CMakeFiles
rmdir /s/q %BUILDDIR%\dcpp
rmdir /s/q %BUILDDIR%\eiskaltdcpp
rmdir /s/q %BUILDDIR%\resources

del /f %BUILDDIR%\Makefile
del /f %BUILDDIR%\CMakeCache.txt
del /f %BUILDDIR%\cmake_install.cmake
del /f %BUILDDIR%\cmake_uninstall.cmake
del /f %BUILDDIR%\CPackConfig.cmake
del /f %BUILDDIR%\CPackSourceConfig.cmake
