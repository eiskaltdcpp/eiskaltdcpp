project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6.3)

set (PROJECT_NAME "EiskaltDC++")
set (APP_VERSION "2.4.2+devel") # Main program version (displayed in UI)
set (CONF_VERSION "2.0402") # Config version (for updating of DCPlusPlus.xml)
set (SO_VERSION "2.4") # Library version (incremented after changes in API or ABI)

if (WIN32 OR APPLE OR HAIKU)
    set (PROJECT_NAME_GLOBAL "${PROJECT_NAME}") # Package name is the same as project name
else ()
    set (PROJECT_NAME_GLOBAL "eiskaltdcpp") # Simplifies packaging in GNU/Linux distros
endif ()

include (CheckCXXSourceCompiles)
include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (FindPkgConfig)
include (TestBigEndian)

CHECK_FUNCTION_EXISTS (mallinfo HAVE_MALLINFO)
CHECK_FUNCTION_EXISTS (malloc_stats HAVE_MALLOC_STATS)
CHECK_FUNCTION_EXISTS (malloc_trim HAVE_MALLOC_TRIM)
CHECK_FUNCTION_EXISTS (daemon HAVE_DAEMON)
CHECK_INCLUDE_FILES ("malloc.h;dlfcn.h;inttypes.h;memory.h;stdlib.h;strings.h;sys/stat.h;limits.h;unistd.h;" FUNCTION_H)
CHECK_INCLUDE_FILES ("sys/socket.h;net/if.h;ifaddrs.h;sys/types.h" HAVE_IFADDRS_H)
CHECK_INCLUDE_FILES ("sys/types.h;sys/statvfs.h;limits.h;stdbool.h;stdint.h" FS_USAGE_C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package (PkgConfig)
find_package (BZip2 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (Threads REQUIRED)
find_package (Gettext REQUIRED)
find_package (Iconv REQUIRED)
find_package (Git)
find_package (Xattr)

option (USE_QT  "Qt 4.x interface" OFF)
option (USE_QT5 "Qt 5.x interface." ON)
option (USE_QT_QML "Build with Qt Declarative Ui support" OFF)
option (USE_QT_SQLITE "Build with SQLite support" ON)
option (USE_GTK  "GTK+ 2.x interface" OFF)
option (USE_GTK3 "GTK+ 3.x interface" OFF)
option (NO_UI_DAEMON "Daemon without any UI" OFF)
option (WITH_DHT "DHT support for libeiskaltdcpp" ON)
option (LUA_SCRIPT "Lua script" ON)
option (USE_MINIUPNP "Use miniupnpc for forward ports" ON)
option (USE_ASPELL "Use aspell for spellchecking" ON)
option (LOCAL_ASPELL_DATA "Use local aspell data files" OFF)
option (USE_LIBGNOME2 "Use LibGnome2 in GTK+ interface (sound notification)" OFF)
option (USE_LIBNOTIFY "Use libnotify in GTK+ interface" ON)
option (USE_PROGRESS_BARS "Use progress bars or plain text labels in Qt UI" ON)
option (FREE_SPACE_BAR_C "Free space progressbar in Qt and GTK+ interface" ON)
option (WITH_EMOTICONS "Install emoticon packs" ON)
option (WITH_EXAMPLES "Install examples sh and others scripts" ON)
option (WITH_LUASCRIPTS "Install examples of lua scripts" ON)
option (WITH_SOUNDS "Install sound files" ON)
option (WITH_DEV_FILES "Install development files (headers for libeiskaltdcpp)" OFF)
option (DBUS_NOTIFY "QtDbus support in Qt interface" ON)
option (USE_JS "QtScript support in Qt interface" OFF)
option (XMLRPC_DAEMON "Make daemon as xmlrpc server" OFF)
option (PERL_REGEX "Use pcre lib for regex in perl style" ON)
option (ENABLE_STACKTRACE "Show stacktrace when program receive SIGSEGV" OFF)
option (USE_IDN2 "Use libidn2 for punycode generation" ON)
option (JSONRPC_DAEMON "Make daemon as jsonrpc server" ON)
option (USE_CLI_XMLRPC "Command line interface to control daemon via XMLRPC" OFF)
option (USE_CLI_JSONRPC "Command line interface to control daemon via JSONRPC" OFF)
option (HAIKU_PKG "Make Haiku package" OFF)
option (CHECK_GTK_DEPRECATED "Check GTK+ backend for deprecated function" OFF)
option (USE_LIBCANBERRA "Use LibCanberra in GTK+ interface (sound notification)" OFF)
option (INSTALL_RUNTIME_PATH "Install rpath" OFF)
option (USE_GOLD "Use ld.gold instead ld.bfd" OFF)
option (USE_CCACHE "Use ccache tool if found" ON)
option (LOCAL_JSONCPP "Use local JsonCpp" ON)
option (BUILD_STATIC "Static build (only for MS Windows and GNU/Linux)" OFF)
option (INSTALL_QT_TRANSLATIONS "Install Qt internal translations" OFF)

if (UNIX)
    option (COMPRESS_MANPAGES "Compress the installed man pages" ON)
endif ()

if (UNIX AND NOT HAIKU)
    option (FORCE_XDG "Respect freedesktop.org standards" ON)
endif ()

if (WIN32)
    option (OPENSSL_MSVC "Use MSVC build openssl (only for MS Windows)" OFF)
    option (INSTALL_DEPENDENCIES "Install all necessary libraries and plugins (only for MS Windows)" OFF)
endif ()

if (USE_QT OR USE_QT5 OR USE_GTK OR USE_GTK3)
    find_package (X11)
endif()

if (PERL_REGEX)
    find_package(PCRE REQUIRED)
endif (PERL_REGEX)

if (USE_IDN2)
    find_package(IDN2 REQUIRED)
endif(USE_IDN2)

if (XATTR_FOUND)
    message (STATUS "Building with libattr support")
endif (XATTR_FOUND)

if (LUA_SCRIPT)
    if (DEFINED LUA_VERSION)
        find_package (Lua ${LUA_VERSION} EXACT REQUIRED)
    else (DEFINED LUA_VERSION)
        find_package (Lua REQUIRED)
    endif (DEFINED LUA_VERSION)
    add_definitions ( -DLUA_SCRIPT )
    message (STATUS "Building with Lua scripts support")
else (LUA_SCRIPT)
    unset (LUA_INCLUDE_DIR CACHE)
    unset (LUA_LIBRARIES CACHE)
    unset (LUA_VERSION_STRING CACHE)
    message (STATUS "Building without Lua scripts support")
endif (LUA_SCRIPT)

if (USE_MINIUPNP)
    message (STATUS "Building with UPnP support (miniupnpc)")
    set (MINIUPNP_INCLUDE_DIR)
    set (MINIUPNP_LIBRARY)
    find_package (Miniupnpc REQUIRED)
else (USE_MINIUPNP)
    unset (MINIUPNP_INCLUDE_DIR CACHE)
    unset (MINIUPNP_LIBRARY CACHE)
    message (STATUS "Building without UPnP support (miniupnpc)")
endif (USE_MINIUPNP)

if (USE_QT OR USE_QT5)
    if (USE_PROGRESS_BARS)
        message (STATUS "Building with Qt UI with progress bars")
    else (USE_PROGRESS_BARS)
        message (STATUS "Building with Qt UI with plain text labels instead of progress bars")
    endif (USE_PROGRESS_BARS)
endif ()

if (FREE_SPACE_BAR_C)
    message (STATUS "Building with free space indicator")
else (FREE_SPACE_BAR_C)
    message (STATUS "Building without free space indicator")
endif (FREE_SPACE_BAR_C)

if (USE_ASPELL AND LOCAL_ASPELL_DATA)
    message (STATUS "LOCAL ASPELL DATA FILES WILL BE USED")
    set (aspell_data_file_name "aspell.tar.gz")
    file(GLOB ASPELL_DATA_TARBALL_EXISTS ${PROJECT_SOURCE_DIR}/data/${aspell_data_file_name})
    if (NOT ASPELL_DATA_TARBALL_EXISTS)
        message (STATUS "Downloading...")
        file(DOWNLOAD https://sf.net/projects/eiskaltdcpp/files/Other/${aspell_data_file_name}
            ${PROJECT_SOURCE_DIR}/data/${aspell_data_file_name} SHOW_PROGRESS)
    endif (NOT ASPELL_DATA_TARBALL_EXISTS)
    file(GLOB ASPELL_DATA_TARBALL_EXISTS ${PROJECT_SOURCE_DIR}/data/${aspell_data_file_name})
    execute_process (COMMAND ${CMAKE_COMMAND} -E tar xf ${ASPELL_DATA_TARBALL_EXISTS}
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/data/)
endif (USE_ASPELL AND LOCAL_ASPELL_DATA)

if (LIB_INSTALL_DIR)
    set (LIBDIR ${LIB_INSTALL_DIR})
endif (LIB_INSTALL_DIR)

if (NOT LIBDIR)
  set (LIBDIR "lib")
endif (NOT LIBDIR)
message (STATUS "Library directory: ${CMAKE_INSTALL_PREFIX}/${LIBDIR}")

if (INSTALL_RUNTIME_PATH)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
endif (INSTALL_RUNTIME_PATH)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "RelWithDebInfo")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug" AND (USE_QT OR USE_QT5))
    message(STATUS "${PROJECT_NAME} Qt with CMAKE_BUILD_TYPE=Debug is very unstable now!\n"
                   "\tYou can build it for your own risk.\n"
                   "\tBut do not even try to write bug reports in this case.\n"
                   "\tPlease use RelWithDebInfo instead it.")
elseif (CMAKE_BUILD_TYPE STREQUAL "Gentoo" AND (USE_QT OR USE_QT5))
    message(STATUS "${PROJECT_NAME} Qt with CMAKE_BUILD_TYPE=Gentoo may be unstable now.\n"
                   "\tPlease try to use RelWithDebInfo instead it if some troubles happend.")
endif (NOT CMAKE_BUILD_TYPE)

if (NOT SHARE_DIR)
    if (WIN32)
        set (SHARE_DIR "resources")
    else ()
        set (SHARE_DIR "share")
    endif ()
endif (NOT SHARE_DIR)

if (APPLE)
    set (CLIENT_DATA_DIR "${PROJECT_NAME_GLOBAL}.app/Contents/Resources/")
    set (CLIENT_DOCS_DIR "${CLIENT_DATA_DIR}/docs")
elseif (WIN32)
    set (CLIENT_DATA_DIR "${SHARE_DIR}")
    set (CLIENT_DOCS_DIR "docs")
elseif (HAIKU)
    set (CLIENT_DATA_DIR ".")
    set (CLIENT_DOCS_DIR "docs")
else ()
    set (CLIENT_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/${PROJECT_NAME_GLOBAL}")
    set (CLIENT_DOCS_DIR "${CLIENT_DATA_DIR}/docs")
endif ()

if (APPLE)
    set (LOCALE_DIR "${CLIENT_DATA_DIR}/locale")
elseif (WIN32)
    set (LOCALE_DIR "${SHARE_DIR}/locale")
elseif (HAIKU)
    set (LOCALE_DIR "locale")
else ()
    set (LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/locale")
endif ()

if (UNIX)
    set (MAN_DIR "${SHARE_DIR}/man/man1")
endif (UNIX)

message (STATUS "Data directory: ${CLIENT_DATA_DIR}")

if (REPLACE_VERSION)
    set (APP_VERSION "${REPLACE_VERSION}")
elseif (GIT_FOUND)
    if (EXISTS "${PROJECT_SOURCE_DIR}/.git/config")
        execute_process (COMMAND ${GIT_EXECUTABLE} describe --tags
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                         OUTPUT_VARIABLE APP_VERSION
                         OUTPUT_STRIP_TRAILING_WHITESPACE)
        string (REGEX REPLACE "^v" "" APP_VERSION "${APP_VERSION}")
        string (REGEX REPLACE "-" "+" APP_VERSION "${APP_VERSION}")
    endif()
endif (REPLACE_VERSION)

string (REGEX REPLACE "^([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" VERSION_BASE "${APP_VERSION}")
message (STATUS "${PROJECT_NAME} full version: ${APP_VERSION}")
message (STATUS "${PROJECT_NAME} base version: ${VERSION_BASE}")

configure_file (${PROJECT_SOURCE_DIR}/Version.h.in ${PROJECT_BINARY_DIR}/VersionGlobal.h)

if (UNIX)
    add_definitions (-D_GNU_SOURCE)
    find_program (GZIP_CMD gzip)
    mark_as_advanced (GZIP_CMD)
endif (UNIX)

if (WIN32 OR APPLE OR HAIKU)
    set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/translations")
else ()
    set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/qt/translations")
endif ()

set (CLIENT_SOUNDS_DEST "${CLIENT_DATA_DIR}/sounds")
set (CLIENT_ICONS_DEST "${CLIENT_DATA_DIR}/icons")

add_definitions (-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT)
add_definitions (-D_DATADIR="${CLIENT_DATA_DIR}")
add_definitions (-DLOCALE_DIR="${LOCALE_DIR}")
add_definitions (-DBUILDING_DCPP)

if (DEBUG)
    add_definitions (-D_DEBUG)
    message (STATUS "Enabled verbose debug output!")
endif (DEBUG)

if (BUILD_STATIC)
    add_definitions (-DBUILD_STATIC -DMINIUPNP_STATICLIB)
endif ()

TEST_BIG_ENDIAN(BIG_ENDIAN)
if (BIG_ENDIAN)
    add_definitions (-DCMAKE_BIG_ENDIAN)
    message(STATUS "Build for BIG_ENDIAN system!")
endif()

if (USE_CCACHE)
    # Configure CCache if available
    find_program (CCACHE_PATH ccache DOC "Path to ccache")
    if (CCACHE_PATH)
        message (STATUS "Found ccache tool at ${CCACHE_PATH}")
        set_property (GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PATH})
        set_property (GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PATH})
    endif ()
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pipe -Wformat -Werror=format-security")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -D_FORTIFY_SOURCE=2")
    if (USE_GOLD)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-linker-plugin")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-linker-plugin")
    endif()
    if (NOT APPLE)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
    endif ()
    if (BUILD_STATIC)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--strip-all")
    endif ()
endif ()

option (LANGUAGES "Using translations")
list (APPEND AllLanguages
            be
            bg
            cs
            da
            de
            el
            en
            es
            eu
            fr
            hu
            it
            pl
            pt_BR
            ru
            sr
            sr@latin
            sk
            tr
            uk
            vi
            zh_CN
            sv_SE
            )
if (NOT DEFINED linguas)
    set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
else (NOT DEFINED linguas)
    if (NOT linguas)
        set (LANGUAGES "" CACHE STRING "Using translations" FORCE)
    elseif (linguas STREQUAL *)
        set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
    else (NOT linguas)
        string(REGEX MATCHALL [a-zA-Z_@]+
            linguas1 ${linguas})
        set (LANGUAGES ${linguas1} CACHE STRING "Using translations" FORCE)
    endif (NOT linguas)
endif (NOT DEFINED linguas)
message (STATUS "Translations: ${LANGUAGES}")

if (WIN32 OR HAIKU)
    set (BINDIR .)
elseif (UNIX)
    set (BINDIR bin)
    if (APPLE)
        set (BUNDLEDIR .)
    endif (APPLE)
endif (WIN32 OR HAIKU)

include_directories(${eiskaltdcpp_BINARY_DIR}
                    ${eiskaltdcpp_SOURCE_DIR}
                    ${GMP_INCLUDE_DIR})

add_subdirectory (data)
add_subdirectory (extra)

if (WITH_DHT)
    add_definitions ( -DWITH_DHT )
    add_subdirectory (dht)
endif (WITH_DHT)

add_subdirectory (dcpp)

if (HAIKU AND HAIKU_PKG)
    add_subdirectory (haiku)
endif ()

if (JSONRPC_DAEMON)
    add_subdirectory (json)
endif (JSONRPC_DAEMON)

if (USE_QT OR USE_QT5)
    add_subdirectory (eiskaltdcpp-qt)
endif (USE_QT OR USE_QT5)

if (USE_GTK OR USE_GTK3)
    add_subdirectory (eiskaltdcpp-gtk)
endif (USE_GTK OR USE_GTK3)

if (NO_UI_DAEMON OR XMLRPC_DAEMON OR JSONRPC_DAEMON)
    add_subdirectory (eiskaltdcpp-daemon)
endif ()

if (USE_CLI_XMLRPC OR USE_CLI_JSONRPC)
    add_subdirectory (eiskaltdcpp-cli)
endif ()

if (WIN32 AND INSTALL_DEPENDENCIES)
    include(${PROJECT_SOURCE_DIR}/windows/install-deps.cmake)
endif ()

if (GETTEXT_FOUND)
    option (UPDATE_PO "Update po files" OFF)
    option (CREATE_MO "Create mo files" ON)
    add_subdirectory (cmake)
endif (GETTEXT_FOUND)

message(STATUS "CMAKE_SYSTEM: ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

if (APPLE)
    message(STATUS "CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
    message(STATUS "CMAKE_OSX_DEPLOYMENT_TARGET: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif (APPLE)

message(STATUS "CMAKE will use these libraries (and headers) during build:

    miniupnpc (headers): ${MINIUPNP_LIBRARY} (${MINIUPNP_INCLUDE_DIR})
    lua (headers): ${LUA_LIBRARIES} (${LUA_INCLUDE_DIR})
    bzip2 (headers): ${BZIP2_LIBRARIES} (${BZIP2_INCLUDE_DIR})
    zlib (headers): ${ZLIB_LIBRARIES} (${ZLIB_INCLUDE_DIR})
    openssl (headers): ${OPENSSL_LIBRARIES} (${OPENSSL_INCLUDE_DIR})
    iconv (headers): ${ICONV_LIBRARIES} (${ICONV_INCLUDE_DIR})
    aspell (headers): ${ASPELL_LIBRARIES} (${ASPELL_INCLUDE_DIR})
    gettext (headers): ${GETTEXT_LIBRARIES} (${GETTEXT_INCLUDE_DIR})
    pcre(cpp) (headers): ${PCRE_LIBRARIES} (${PCRE_INCLUDE_DIR})
    idn2 (headers): ${IDN2_LIBRARIES} (${IDN2_INCLUDE_DIR})
    ")

configure_file("${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake")
