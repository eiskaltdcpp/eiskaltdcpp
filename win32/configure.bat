call variables.bat

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" -DSHARE_DIR=resources -DCMAKE_FIND_ROOT_PATH="%MINGW32_DIR%";"%ASPELL_DIR%";"%BOOST_DIR%";"%GETTEXT_DIR%";"%LIBBZ2_DIR%";"%LIBICONV_DIR%";"%LIBIDN_DIR%";"%LIBZ_DIR%";"%LUA_DIR%" -DUSE_ASPELL=ON -DDBUS_NOTIFY=OFF -DFORCE_XDG=OFF -DUSE_MINIUPNP=ON -DLOCAL_MINIUPNP=ON -DLUA_SCRIPT=ON -DWITH_LUASCRIPTS=ON -DUSE_JS=ON -DUSE_QT_QML=ON -DUSE_QT_SQLITE=ON -DWITH_SOUNDS=ON -DNO_UI_DAEMON=ON -DJSONRPC_DAEMON=ON -DCMAKE_CXX_FLAGS="-I%BOOST_HEADERS_DIR% -I%GETTEXT_HEADERS_DIR%" "%SOURCES_DIR%"
