call variables.bat

rmdir /s/q %BUILD_DIR%\CMakeFiles
rmdir /s/q %BUILD_DIR%\cmake
rmdir /s/q %BUILD_DIR%\data
rmdir /s/q %BUILD_DIR%\dcpp
rmdir /s/q %BUILD_DIR%\dht
rmdir /s/q %BUILD_DIR%\eiskaltdcpp-cli
rmdir /s/q %BUILD_DIR%\eiskaltdcpp-daemon
rmdir /s/q %BUILD_DIR%\eiskaltdcpp-qt
rmdir /s/q %BUILD_DIR%\extra
rmdir /s/q %BUILD_DIR%\json
rmdir /s/q %BUILD_DIR%\upnp

del /f %BUILD_DIR%\CMakeCache.txt
del /f %BUILD_DIR%\CPackConfig.cmake
del /f %BUILD_DIR%\CPackSourceConfig.cmake
del /f %BUILD_DIR%\Makefile
del /f %BUILD_DIR%\VersionGlobal.h
del /f %BUILD_DIR%\cmake_install.cmake
del /f %BUILD_DIR%\cmake_uninstall.cmake
del /f %BUILD_DIR%\install_manifest.txt
