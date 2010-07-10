call "variables.bat"

echo "#define BOOST_VERSION 000000" > "%MINGW%\include\boost\version.hpp"

cmake -G "MinGW Makefiles" -DCMAKE_FIND_ROOT_PATH="%MINGW%" -DUSE_ASPELL=OFF -DUSE_LIBUPNP=OFF -DDBUS_NOTIFY=OFF -DFORCE_XDG=OFF -DFREE_SPACE_BAR_C=ON ..

