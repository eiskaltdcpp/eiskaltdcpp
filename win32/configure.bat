call variables.bat

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%INSTALLDIR% -DSHARE_DIR=resources -DCMAKE_FIND_ROOT_PATH=%MINGW% -DUSE_ASPELL=ON -DDBUS_NOTIFY=OFF -DFORCE_XDG=OFF -DUSE_MINIUPNP=ON -DLOCAL_MINIUPNP=ON -DLUA_SCRIPT=OFF -DWITH_LUASCRIPTS=OFF -DUSE_JS=ON -DUSE_QT_QML=ON -DWITH_SOUNDS=ON %SOURCESDIR%

@rem Then using cross-compiled or static openssl append the parameters below to fool CMake building system.
@rem -DLIB_EAY:FILEPATH=crypto -DSSL_EAY:FILEPATH=ssl
