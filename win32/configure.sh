#!/bin/sh

source ./variables.sh

cmake -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_FIND_ROOT_PATH=$QT_MINGW32_DIR\;$OPENSSL_DIR\;$ASPELL_DIR\;$BOOST_DIR\;$GETTEXT_DIR\;$LIBBZ2_DIR\;$LIBICONV_DIR\;$LIBIDN_DIR\;$LIBZ_DIR\;$LUA_DIR\;$MINGW32_DIR -DCMAKE_C_COMPILER=$MINGW32_NAME-gcc -DCMAKE_CXX_COMPILER=$MINGW32_NAME-g++ -DCMAKE_RC_COMPILER=/usr/bin/i486-mingw32-windres -DCMAKE_CXX_FLAGS="-I$BOOST_HEADERS_DIR -I$GETTEXT_HEADERS_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DSHARE_DIR=resources -DUSE_ASPELL=ON -DFORCE_XDG=OFF -DDBUS_NOTIFY=OFF -DUSE_JS=ON -DUSE_MINIUPNP=ON -DLOCAL_MINIUPNP=ON -DWITH_SOUNDS=ON -DPERL_REGEX=OFF -DUSE_QT_QML=ON -DLUA_SCRIPT=ON -DWITH_LUASCRIPTS=ON -DUSE_QT_SQLITE=ON -DNO_UI_DAEMON=ON -DJSONRPC_DAEMON=ON $SOURCES_DIR
