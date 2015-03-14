#/bin/sh

. ./variables.sh

rm -rf $BUILD_DIR/CMakeFiles
rm -rf $BUILD_DIR/cmake
rm -rf $BUILD_DIR/data
rm -rf $BUILD_DIR/dcpp
rm -rf $BUILD_DIR/dht
rm -rf $BUILD_DIR/eiskaltdcpp-cli
rm -rf $BUILD_DIR/eiskaltdcpp-daemon
rm -rf $BUILD_DIR/eiskaltdcpp-qt
rm -rf $BUILD_DIR/extra
rm -rf $BUILD_DIR/json
rm -rf $BUILD_DIR/upnp

rm -f $BUILD_DIR/CMakeCache.txt
rm -f $BUILD_DIR/CPackConfig.cmake
rm -f $BUILD_DIR/CPackSourceConfig.cmake
rm -f $BUILD_DIR/Makefile
rm -f $BUILD_DIR/VersionGlobal.h
rm -f $BUILD_DIR/cmake_install.cmake
rm -f $BUILD_DIR/cmake_uninstall.cmake
rm -f $BUILD_DIR/install_manifest.txt
