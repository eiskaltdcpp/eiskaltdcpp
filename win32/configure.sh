#!/bin/sh
cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain-mingw32.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=. -DSHARE_DIR=resources -DUSE_ASPELL=ON -DFORCE_XDG=OFF -DDBUS_NOTIFY=OFF -DUSE_JS=ON -DUSE_MINIUPNP=ON -DLOCAL_MINIUPNP=ON -DLUA_SCRIPT=ON -DWITH_LUASCRIPTS=ON -DWITH_SOUNDS=ON -DPERL_REGEX=OFF -DUSE_QT_QML=ON -DNO_UI_DAEMON=ON -DUSE_QT_SQLITE=ON /path/to/eiskaltdcpp/

#Then using cross-compiled and static openssl append the parameters below to fool CMake building system.
#-DLIB_EAY:FILEPATH=crypto -DSSL_EAY:FILEPATH=ssl

