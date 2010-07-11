call "variables.bat"

IF NOT EXIST "%MINGW%\include\boost\version.hpp" echo "#define BOOST_VERSION 000000" >"%MINGW%\include\boost\version.hpp"

cmake -G "MinGW Makefiles" -DCMAKE_FIND_ROOT_PATH="%MINGW%" -DPROJECT_NAME_GLOBAL=EiskaltDC++ -DSHARE_DIR=stuff -DUSE_ASPELL=OFF -DUSE_LIBUPNP=OFF -DDBUS_NOTIFY=OFF -DFORCE_XDG=OFF -DFREE_SPACE_BAR_C=ON ..

