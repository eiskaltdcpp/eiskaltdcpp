call variables.bat

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%INSTALLDIR% -DSHARE_DIR=resources -DCMAKE_FIND_ROOT_PATH=%MINGW% -DUSE_ASPELL=OFF -DDBUS_NOTIFY=OFF -DFORCE_XDG=OFF -DFREE_SPACE_BAR_C=ON -DLOCAL_MINIUPNP=ON -DLUA_SCRIPT=ON -DWITH_LUASCRIPTS=ON -DUSE_JS=ON -DWITH_SOUNDS=ON %SOURCESDIR%

@rem Then using cross-compiled or static openssl append the parameters below to fool CMake building system.
@rem -DLIB_EAY:FILEPATH=crypto -DSSL_EAY:FILEPATH=ssl
