project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6.3)

if (APPLE)
  set (PROJECT_NAME_GLOBAL EiskaltDC++)
else (APPLE)
  set (PROJECT_NAME_GLOBAL eiskaltdcpp)
endif (APPLE)

set (PROJECT_NAME "EiskaltDC++")
set (VERSION "2.3.0")
set (SOVERSION "2.3")
set (VERSION_SUFFIX "GIT")

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (FindPkgConfig)
include (CheckCXXSourceCompiles)

#IF (GPTOOLS_CPU AND GPTOOLS_MEM)
#    SET (GPTOOLS_CPU FALSE)
#    SET (GPTOOLS_MEM FALSE)
#    SET (GPTOOLS_CPU_AND_MEM TRUE)
#ENDIF (GPTOOLS_CPU AND GPTOOLS_MEM)
#IF (GPTOOLS_MEM)
#    MESSAGE (STATUS "Enabled memory profiling via Google perftools")
#    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
#    SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc")
#ENDIF (GPTOOLS_MEM)
#IF (GPTOOLS_CPU)
#    MESSAGE (STATUS "Enabled CPU profiling via Google perftools")
#    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
#    SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
#ENDIF (GPTOOLS_CPU)
#IF (GPTOOLS_CPU_AND_MEM)
#    MESSAGE (STATUS "Enabled both CPU and memory profiling via Google perftools")
#    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc_and_profiler")
#    SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc_and_profiler")
#ENDIF (GPTOOLS_CPU_AND_MEM)

CHECK_FUNCTION_EXISTS(mallinfo HAVE_MALLINFO)
CHECK_FUNCTION_EXISTS(malloc_stats HAVE_MALLOC_STATS)
CHECK_FUNCTION_EXISTS(malloc_trim HAVE_MALLOC_TRIM)
CHECK_FUNCTION_EXISTS(daemon HAVE_DAEMON)
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

option (USE_QT  "Qt 4.x interface" ON)
option (USE_QT5 "Qt 5.x interface." OFF)
option (USE_QT_QML "Build with Qt Declarative Ui support" OFF)
option (USE_QT_SQLITE "Build with SQLite support" OFF)
option (USE_GTK  "GTK+ 2.x interface" OFF)
option (USE_GTK3 "GTK+ 3.x interface" OFF)
option (NO_UI_DAEMON "Daemon without any UI" OFF)
option (WITH_DHT "DHT support for libeiskaltdcpp" ON)
option (LUA_SCRIPT "Lua script" OFF)
option (USE_MINIUPNP "Use miniupnpc for forward ports" OFF)
option (LOCAL_MINIUPNP "Use local miniupnpc" OFF)
option (USE_ASPELL "Use aspell for spellchecking" OFF)
option (FORCE_XDG "Respect freedesktop.org standards" ON)
option (USE_LIBGNOME2 "Use LibGnome2 in GTK+ interface (sound notification)" OFF)
option (USE_LIBNOTIFY "Use libnotify in GTK+ interface" ON)
option (FREE_SPACE_BAR_C "Free space progressbar in Qt and GTK+ interface" ON)
option (WITH_EMOTICONS "Install emoticon packs" ON)
option (WITH_EXAMPLES "Install examples sh and others scripts" ON)
option (WITH_LUASCRIPTS "Install examples of lua scripts" OFF)
option (WITH_SOUNDS "Install sound files" OFF)
option (WITH_DEV_FILES "Install development files (headers for libeiskaltdcpp)" OFF)
option (DBUS_NOTIFY "QtDbus support in Qt interface" ON)
option (USE_JS "QtScript support in Qt interface")
option (XMLRPC_DAEMON "Make daemon as xmlrpc server" OFF)
option (PERL_REGEX "Use pcre lib for regex in perl style" OFF)
option (ENABLE_STACKTRACE "Show stacktrace when program receive SIGSEGV" OFF)
option (USE_IDNA "Use libidn for punycode generation" ON)
option (JSONRPC_DAEMON "Make daemon as jsonrpc server" OFF)
option (USE_CLI_XMLRPC "Command line interface to control daemon via XMLRPC" OFF)
option (USE_CLI_JSONRPC "Command line interface to control daemon via JSONRPC" OFF)
option (HAIKU_PKG "Make Haiku package" OFF)
option (CHECK_GTK_DEPRECATED "Check GTK+ backend for deprecated function" OFF)
option (USE_LIBCANBERRA "Use LibCanberra in GTK+ interface (sound notification)" OFF)
option (INSTALL_RUNTIME_PATH "Install rpath" OFF)
option (USE_GOLD "Use ld.gold instead ld.bfd" OFF)
option (DO_NOT_USE_MUTEX "Do not use mutex (necessary for Haiku and WINE)" OFF)
option (LOCAL_JSONCPP "Use local JsonCpp" ON)
option (LOCAL_BOOST "Use local boost headers" OFF)
option (MINIUPNP_WITH_IPV6 "Try use upnp for ipv6" OFF)

if (DO_NOT_USE_MUTEX OR HAIKU OR APPLE)
  add_definitions ( -DDO_NOT_USE_MUTEX )
endif ()

if (USE_QT OR USE_QT5 OR USE_GTK OR USE_GTK3)
    find_package (X11)
endif()

if (PERL_REGEX)
    find_package(Pcre REQUIRED)
endif (PERL_REGEX)

if (USE_IDNA)
    find_package(IDNA REQUIRED)
endif(USE_IDNA)

if (XATTR_FOUND)
    message (STATUS "Building with libattr support")
endif (XATTR_FOUND)

if (LUA_SCRIPT)
    find_package (Lua REQUIRED)
    add_definitions ( -DLUA_SCRIPT )
    message (STATUS "Building with Lua scripts support")
else (LUA_SCRIPT)
    unset (LUA_INCLUDE_DIR CACHE)
    unset (LUA_LIBRARIES CACHE)
    unset (LUA_VERSION_STRING CACHE)
    message (STATUS "Building without Lua scripts support")
endif(LUA_SCRIPT)

if (USE_MINIUPNP)
    message (STATUS "Building with UPNP support (miniupnpc)")
    set (MINIUPNP_INCLUDE_DIR)
    set (MINIUPNP_LIBRARY)
  if (LOCAL_MINIUPNP)
    add_subdirectory(upnp)
    set (MINIUPNP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/upnp")
    set (MINIUPNP_LIBRARY "miniupnpc")
  else (LOCAL_MINIUPNP)
    find_package (Miniupnpc REQUIRED)
  endif (LOCAL_MINIUPNP)

elseif (NOT USE_MINIUPNP)
    unset (MINIUPNP_INCLUDE_DIR CACHE)
    unset (MINIUPNP_LIBRARY CACHE)
    message (STATUS "Building without UPNP support (miniupnpc)")
endif(USE_MINIUPNP)

if (FREE_SPACE_BAR_C)
    message (STATUS "Building with free space bar (realisation on C, POSIX 1003.1-2001 (and later) with XSI)")
else (FREE_SPACE_BAR_C)
    message (STATUS "Building without free space bar")
endif (FREE_SPACE_BAR_C)

if (LOCAL_BOOST)
set (boost_file_name "boost.tar.gz")
file(GLOB BOOST_TAR_EXISTS ${PROJECT_BINARY_DIR}/${boost_file_name})
if (NOT BOOST_TAR_EXISTS)
file(DOWNLOAD http://sf.net/projects/eiskaltdcpp/files/Other/${boost_file_name} ${PROJECT_BINARY_DIR}/${boost_file_name})
endif (NOT BOOST_TAR_EXISTS)
file(GLOB BOOST_TAR_EXISTS ${PROJECT_BINARY_DIR}/${boost_file_name})
find_program (TAR_CMD tar)
mark_as_advanced(TAR_CMD)
execute_process (COMMAND ${TAR_CMD} -xf ${BOOST_TAR_EXISTS}
                 WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/)
set (Boost_INCLUDE_DIR ${PROJECT_BINARY_DIR})
if (HAIKU)
EXECUTE_PROCESS (
                COMMAND patch -p0 -i ${PROJECT_SOURCE_DIR}/haiku/boost.diff
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/)
endif()
message (STATUS "USED LOCAL BOOST HEADERS")
else (LOCAL_BOOST)
    find_package (Boost REQUIRED)
    if (Boost_VERSION VERSION_LESS 104200)
        add_definitions(-DTIMER_OLD_BOOST)
    endif()
    if (Boost_VERSION VERSION_GREATER 105000)
        find_package(Boost COMPONENTS system REQUIRED)
    endif()
    if (Boost_VERSION VERSION_LESS 105300 AND WIN32)
        add_definitions(-DFIX_FOR_OLD_BOOST)
    endif()
endif(LOCAL_BOOST)

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
  message(STATUS "EiskaltDC++ Qt with CMAKE_BUILD_TYPE=Debug is very unstable now!\n"
                 "\tYou can build it for your own risk.\n"
                 "\tBut do not even try to write bug reports in this case.\n"
                 "\tPlease use RelWithDebInfo instead it.")
elseif (CMAKE_BUILD_TYPE STREQUAL "Gentoo" AND (USE_QT OR USE_QT5))
  message(STATUS "EiskaltDC++ Qt with CMAKE_BUILD_TYPE=Gentoo may be unstable now.\n"
                 "\tPlease try to use RelWithDebInfo instead it if some troubles happend.")
endif (NOT CMAKE_BUILD_TYPE)

if (NOT SHARE_DIR)
set (SHARE_DIR "share")
endif (NOT SHARE_DIR)

if (APPLE)
    add_definitions(-DAPPLE)
endif (APPLE)

if (APPLE)
  set (CLIENT_DATA_DIR "${PROJECT_NAME_GLOBAL}.app")
elseif (WIN32 OR HAIKU)
  set (CLIENT_DATA_DIR "${SHARE_DIR}")
else (APPLE)
  set (CLIENT_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/${PROJECT_NAME_GLOBAL}")
endif (APPLE)

if (APPLE)
  set (LOCALE_DIR "${PROJECT_NAME_GLOBAL}.app/locale")
elseif (WIN32 OR HAIKU)
  set (LOCALE_DIR "${SHARE_DIR}/locale")
else (APPLE)
  set (LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/locale")
endif (APPLE)

if (APPLE)
  set (MAN_DIR "${PROJECT_NAME_GLOBAL}.app/man/man1")
else (APPLE)
  set (MAN_DIR "${SHARE_DIR}/man/man1")
endif (APPLE)

message (STATUS "Data directory: ${CLIENT_DATA_DIR}")

if (DCPP_REVISION)
    set (eiskaltdcpp_WC_REVISION ${DCPP_REVISION})
    message (STATUS "DCPP_REVISION is set to: ${eiskaltdcpp_WC_REVISION}")
elseif (GIT_FOUND)
    execute_process (COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                OUTPUT_VARIABLE eiskaltdcpp_WC_REVISION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
    message (STATUS "Build Git commit ID: ${eiskaltdcpp_WC_REVISION}")
    if (NOT eiskaltdcpp_WC_REVISION MATCHES [0-9a-z]+)
        set (eiskaltdcpp_WC_REVISION "${VERSION_SUFFIX}")
    endif ()
elseif (NOT DCPP_REVISION)
    set (eiskaltdcpp_WC_REVISION "${VERSION_SUFFIX}")
endif ()

configure_file(${PROJECT_SOURCE_DIR}/Version.h.in ${PROJECT_BINARY_DIR}/VersionGlobal.h)

if (UNIX)
add_definitions (-D_GNU_SOURCE)
find_program (GZIP_CMD gzip)
mark_as_advanced(GZIP_CMD)
endif (UNIX)

set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/qt/ts")
set (CLIENT_SOUNDS_DEST "${CLIENT_DATA_DIR}/sounds")
set (CLIENT_ICONS_DEST "${CLIENT_DATA_DIR}/icons")
set (CLIENT_SCRIPTS_DEST "${CLIENT_DATA_DIR}/qt/qtscripts")

add_definitions (-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT
                 -D_DATADIR="${CLIENT_DATA_DIR}" -DLOCALE_DIR="${LOCALE_DIR}" -DBUILDING_DCPP)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    if (MINGW AND NOT CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-auto-import")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
    else ()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pipe -Wformat -Werror=format-security")
    endif ()
    if (USE_GOLD)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-linker-plugin")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-linker-plugin")
    endif()
    if (NOT APPLE)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
    endif ()
endif ()

include_directories(${eiskaltdcpp_BINARY_DIR}
   ${eiskaltdcpp_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   )

option (LANGUAGES "Using translations")
list (APPEND AllLanguages
            be
            bg
            cs
            de
            el
            en
            es
            fr
            hu
            it
            pl
            pt_BR
            ru
            sr@latin
            sk
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

add_subdirectory(extra)
add_subdirectory(data)

if (WITH_DHT)
  add_subdirectory (dht)
endif (WITH_DHT)

add_subdirectory (dcpp)

if (HAIKU AND HAIKU_PKG)
  add_subdirectory (haiku)
endif ()

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

if (JSONRPC_DAEMON)
add_subdirectory (json)
endif (JSONRPC_DAEMON)

if(GETTEXT_FOUND)
    option (UPDATE_PO "Update po files" OFF)
    option (CREATE_MO "Create mo files" ON)
    add_subdirectory(cmake)
endif(GETTEXT_FOUND)

message(STATUS "CMAKE_SYSTEM: ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE will use this libs (and headers) during build:

    miniupnpc (headers):${MINIUPNP_LIBRARY} (${MINIUPNP_INCLUDE_DIR})
    lua (headers): ${LUA_LIBRARIES} (${LUA_INCLUDE_DIR})
    boost (headers): ${Boost_LIBRARIES} (${Boost_INCLUDE_DIR})
    bzip2 (headers): ${BZIP2_LIBRARIES} (${BZIP2_INCLUDE_DIR})
    zlib (headers): ${ZLIB_LIBRARIES} (${ZLIB_INCLUDE_DIR})
    openssl (headers): ${OPENSSL_LIBRARIES} (${OPENSSL_INCLUDE_DIR})
    iconv (headers): ${ICONV_LIBRARIES} (${ICONV_INCLUDE_DIR})
    aspell (headers): ${ASPELL_LIBRARIES} (${ASPELL_INCLUDE_DIR})
    gettext (headers): ${GETTEXT_LIBRARIES} (${GETTEXT_INCLUDE_DIR})
    pcre(cpp) (headers): ${PCRE_LIBRARIES} (${PCRE_INCLUDE_DIR})
    idna (headers): ${IDNA_LIBRARIES} (${IDNA_INCLUDE_DIR})
    ")

CONFIGURE_FILE(
  "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake")
