project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6.3)

if (APPLE)
  set (PROJECT_NAME_GLOBAL EiskaltDC++)
else (APPLE)
  set (PROJECT_NAME_GLOBAL eiskaltdcpp)
endif (APPLE)

set (PROJECT_NAME "EiskaltDC++")
set (VERSION "2.2.5")
set (SOVERSION "2.2")
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

CHECK_INCLUDE_FILE (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILE (dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE (inttypes.h HAVE_INTTYPES_H)
CHECK_FUNCTION_EXISTS(mallinfo HAVE_MALLINFO)
CHECK_FUNCTION_EXISTS(malloc_stats HAVE_MALLOC_STATS)
CHECK_FUNCTION_EXISTS(malloc_trim HAVE_MALLOC_TRIM)
CHECK_FUNCTION_EXISTS(daemon HAVE_DAEMON)
CHECK_INCLUDE_FILE (memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE (stdbool.h HAVE_STDBOOL_H)
CHECK_INCLUDE_FILE (strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE (sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE (limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILE (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE (sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE (ifaddrs.h HAVE_IFADDRS_H)
CHECK_INCLUDE_FILE (net/if.h HAVE_NET_IF_H)
CHECK_INCLUDE_FILE (sys/statvfs.h STAT_STATVFS)
CHECK_INCLUDE_FILES ("sys/socket.h;net/if.h;ifaddrs.h;sys/types.h" HAVE_ADDRS_H)
CHECK_INCLUDE_FILES ("sys/types.h;sys/statvfs.h;limits.h;stdbool.h;stdint.h" FS_USAGE_C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (PkgConfig)
find_package (BZip2 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (Threads REQUIRED)
find_package (Gettext REQUIRED)
find_package (Iconv REQUIRED)
find_package (Subversion)
find_package (Git)

option (USE_QT "Qt interface" ON)
option (USE_QT_QML "Build with Qt Declarative Ui support" OFF)
option (USE_QT_SQLITE "Build with SQLite support" OFF)
option (USE_GTK "GTK interface" OFF)
option (WITH_DHT "DHT support for libeiskaltdcpp" ON)
option (LUA_SCRIPT "Lua script" OFF)
option (USE_MINIUPNP "Use miniupnpc for forward ports" OFF)
option (LOCAL_MINIUPNP "Use local miniupnpc" OFF)
option (USE_ASPELL "Use aspell for spellchecking")
option (FORCE_XDG "Respect freedesktop.org standards" ON)
option (USE_LIBGNOME2 "Use LibGnome2 in GTK interface" OFF)
option (USE_LIBNOTIFY "Use libnotify in GTK interface" ON)
option (FREE_SPACE_BAR_C "Free space progressbar in Qt interface" ON)
option (WITH_EMOTICONS "Install emoticon packs" ON)
option (WITH_EXAMPLES "Install examples sh and others scripts" ON)
option (WITH_LUASCRIPTS "Install examples of lua scripts" OFF)
option (WITH_SOUNDS "Install sound files" OFF)
option (WITH_DEV_FILES "Install development files (headers for libeiskaltdcpp)" OFF)
option (DBUS_NOTIFY "QtDbus support in Qt interface" ON)
option (USE_JS "QtScript support in Qt interface")
option (NO_UI_DAEMON "Daemon without any UI" OFF)
option (XMLRPC_DAEMON "Make daemon as xmlrpc server" OFF)
option (PERL_REGEX "Use pcre lib for regex in perl style" OFF)
option (ENABLE_STACKTRACE "Show stacktrace when program receive SIGSEGV" OFF)
option (USE_IDNA "Use libidn for punycode generation" ON)
option (JSONRPC_DAEMON "Make daemon as jsonrpc server" OFF)
option (USE_GTK3 "GTK3 interface" OFF)
option (USE_CLI "Command line interface" OFF)
option (HAIKU_PKG "Make Haiku package" OFF)
option (CHECK_GTK_DEPRECATED "Check gtk backend for deprecated function" OFF)

if (USE_QT OR USE_GTK)
    find_package (X11)
endif(USE_QT OR USE_GTK)

if (PERL_REGEX)
    find_package(Pcre REQUIRED)
endif (PERL_REGEX)

if (USE_IDNA)
    find_package(IDNA REQUIRED)
endif(USE_IDNA)

if (LUA_SCRIPT)
    find_package (Lua51 REQUIRED)
    add_definitions ( -DLUA_SCRIPT )
    message (STATUS "Building with Lua scripts support")
else (LUA_SCRIPT)
    set (LUA_INCLUDE_DIR )
    set (LUA_LIBRARIES )
    message (STATUS "Building without Lua scripts support")
endif(LUA_SCRIPT)

if (USE_MINIUPNP)
    message (STATUS "Building with UPNP support (miniupnpc)")
    set (MINIUPNP_INCLUDE_DIR)
    set (MINIUPNP_LIBRARY)
  if (LOCAL_MINIUPNP)
    add_subdirectory(upnp)
    set (MINIUPNP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/upnp")
    set (MINIUPNP_LIBRARY "miniupnpc")
  else (LOCAL_MINIUPNP)
    find_package (Miniupnpc REQUIRED)
  endif (LOCAL_MINIUPNP)

elseif (NOT USE_MINIUPNP)
    unset (MINIUPNP_INCLUDE_DIR CACHE)
    unset (MINIUPNP_LIBRARY CACHE)
    message (STATUS "Building without UPNP support (miniupnpc)")
endif(USE_MINIUPNP)

if (USE_QT)
    if (USE_QT_QML)
      find_package (Qt4 4.7.0 COMPONENTS QtCore QtGui QtXml QtNetwork QtDeclarative REQUIRED)
      message (STATUS "Building with Qt Declarative UI support")
      set (USE_JS ON)
    else (USE_QT_QML)
      find_package (Qt4 4.4.0 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED)
      message (STATUS "Building without Qt Declarative UI support")
    endif (USE_QT_QML)

    if (DBUS_NOTIFY)
        find_package (Qt4 4.4.0 COMPONENTS QtDBus REQUIRED)
        message (STATUS "Building with Qt DBus support")
    else (DBUS_NOTIFY)
        message (STATUS "Building without Qt DBus support")
    endif (DBUS_NOTIFY)

    if (USE_JS)
        find_package (Qt4 4.4.0 COMPONENTS QtScript REQUIRED)
	message (STATUS "Building with Qt Script support")
    else (USE_JS)
        message (STATUS "Building without Qt Script support")
    endif (USE_JS)

    if (USE_QT_SQLITE)
        find_package (Qt4 4.4.0 COMPONENTS QtSql REQUIRED)
        message (STATUS "Building with Qt SQLite support")
    else (USE_QT_SQLITE)
        message (STATUS "Building without Qt SQLite support")
    endif (USE_QT_SQLITE)
endif (USE_QT)


if (NOT USE_QT)
    set (USE_ASPELL OFF)
endif (NOT USE_QT)

if (NOT USE_ASPELL)
    if(USE_ASPELL EQUAL 0)
    else(USE_ASPELL EQUAL 0)
        find_package (ASPELL)
        if (ASPELL_FOUND)
            set (USE_ASPELL 1 CACHE BOOL "Use aspell for spellchecking" FORCE)
        endif (ASPELL_FOUND)
    endif(USE_ASPELL EQUAL 0)
endif (NOT USE_ASPELL)

if (USE_ASPELL)
    find_package (ASPELL REQUIRED)
    message (STATUS "Building with Aspell support")
else (USE_ASPELL)
    message (STATUS "Building without Aspell support")
endif (USE_ASPELL)

if (FREE_SPACE_BAR_C)
    message (STATUS "Building with free space bar (realisation on C, POSIX 1003.1-2001 (and later) with XSI)")
else (FREE_SPACE_BAR_C)
    message (STATUS "Building without free space bar")
endif (FREE_SPACE_BAR_C)

if (USE_GTK OR USE_GTK3)

    if (USE_GTK)
        find_package (GTK2 REQUIRED gtk)
    elseif (USE_GTK3)
        find_package (GTK3 REQUIRED gtk)
    endif (USE_GTK)

    if (USE_LIBGNOME2)
        message (STATUS "Building with LibGnome2 support")
        find_package (LibGnome2 REQUIRED)
    else (USE_LIBGNOME2)
        message (STATUS "Building without LibGnome2 support")
    endif (USE_LIBGNOME2)

    if (USE_LIBNOTIFY)
        message (STATUS "Building with libnotify support")
        set (LIBNOTIFY "-lnotify")
    else (USE_LIBNOTIFY)
        message (STATUS "Building without libnotify support")
    endif (USE_LIBNOTIFY)
    if (USE_GTK)
        if (GTK2_VERSION VERSION_LESS 2.24.0)
            file(GLOB gtk2_ph eiskaltdcpp-gtk/patches/*.diff)
            foreach (ph ${gtk2_ph})
                EXECUTE_PROCESS (
                    COMMAND patch -p1 -i ${ph}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
            endforeach()
        endif(GTK2_VERSION VERSION_LESS 2.24.0)
    endif (USE_GTK)
endif (USE_GTK OR USE_GTK3)

if (LOCAL_BOOST)
file(GLOB BOOST_TAR_EXISTS  *.tar.gz)
if (NOT BOOST_TAR_EXISTS)
set (boost_file_name "boost.tar.gz")
file(DOWNLOAD http://eiskaltdc.googlecode.com/files/${boost_file_name} ${CMAKE_CURRENT_SOURCE_DIR}/${boost_file_name} LOG boost_logs STATUS boost_status)
message (STATUS "${boost_logs}")
message (STATUS "${boost_status}")
endif (NOT BOOST_TAR_EXISTS)
file(GLOB BOOST_TAR_EXISTS *.tar.gz)
FIND_PROGRAM (TAR_CMD tar)
  EXECUTE_PROCESS (
                COMMAND ${TAR_CMD} -xf ${BOOST_TAR_EXISTS}
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/)
set (Boost_INCLUDE_DIR ${PROJECT_BINARY_DIR})
if (HAIKU)
EXECUTE_PROCESS (
                COMMAND patch -p0 -i haiku/boost.diff
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()
message (STATUS "USED LOCAL BOOST HEADERS")
else (LOCAL_BOOST)
    find_package (Boost REQUIRED)
    if (Boost_VERSION VERSION_LESS 103900)
        add_definitions(-DFIX_FOR_OLD_BOOST)
    endif()
    if (Boost_VERSION VERSION_LESS 104200)
        add_definitions(-DTIMER_OLD_BOOST)
    endif()
endif(LOCAL_BOOST)

if (NOT CMAKE_INSTALL_PREFIX)
  set (CMAKE_INSTALL_PREFIX "/usr")
endif (NOT CMAKE_INSTALL_PREFIX)

if (LIB_INSTALL_DIR)
  set (LIBDIR ${LIB_INSTALL_DIR})
endif (LIB_INSTALL_DIR)

if (NOT LIBDIR)
  set (LIBDIR "lib")
endif (NOT LIBDIR)
message (STATUS "Library directory: ${CMAKE_INSTALL_PREFIX}/${LIBDIR}")

if (WITH_DEV_FILES)
  if (NOT EISKALTDCPP_INCLUDE_DIR)
    set (EISKALTDCPP_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/eiskaltdcpp")
  endif (NOT EISKALTDCPP_INCLUDE_DIR)
  message (STATUS "Headers of libeiskaltdcpp will be installed to: ${EISKALTDCPP_INCLUDE_DIR}")
endif (WITH_DEV_FILES)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug" AND USE_QT)
  message(STATUS "EiskaltDC++ Qt with CMAKE_BUILD_TYPE=Debug is very unstable now!\n"
                 "\tYou can build it for your own risk.\n"
                 "\tBut do not even try to write bug reports in this case.\n"
                 "\tPlease use RelWithDebInfo instead it.")
elseif (CMAKE_BUILD_TYPE STREQUAL "Gentoo" AND USE_QT)
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
elseif (Subversion_FOUND)
    Subversion_WC_INFO (${PROJECT_SOURCE_DIR} eiskaltdcpp)
    message (STATUS "Build SVN revision: ${eiskaltdcpp_WC_REVISION}")
    if (eiskaltdcpp_WC_REVISION MATCHES [0-9]+)
        set (eiskaltdcpp_WC_REVISION "r${eiskaltdcpp_WC_REVISION_TEMP}")
    else ()
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
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wformat -Werror=format-security")
    endif ()
    if (NOT APPLE AND NOT CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--as-needed")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed")
    endif ()
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pipe -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE     "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG -pipe")
    set (CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -UNDEBUG -g3 -pipe")
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
            en
            es
            fr
            hu
            pl
            ru
            sr@latin
            sk
            uk
            )
if (NOT DEFINED linguas)
  set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
else (NOT DEFINED linguas)
  if (NOT linguas)
    set (LANGUAGES "" CACHE STRING "Using translations" FORCE)
  elseif (linguas STREQUAL *)
        set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
  else (NOT linguas)
    string(REGEX MATCHALL [a-zA-Z_]+
           linguas1 ${linguas})
    set (LANGUAGES ${linguas1} CACHE STRING "Using translations" FORCE)
  endif (NOT linguas)
endif (NOT DEFINED linguas)
message (STATUS "Translations: ${LANGUAGES}")

add_subdirectory(extra)
add_subdirectory(data)

if (WITH_DHT)
  add_subdirectory (dht)
endif (WITH_DHT)

add_subdirectory (dcpp)

if (USE_QT)
  add_subdirectory (eiskaltdcpp-qt)
endif (USE_QT)

if (USE_GTK OR USE_GTK3)
  add_subdirectory (eiskaltdcpp-gtk)
endif (USE_GTK OR USE_GTK3)

if (NO_UI_DAEMON OR XMLRPC_DAEMON OR JSONRPC_DAEMON)
  add_subdirectory (eiskaltdcpp-daemon)
endif ()

if (USE_CLI)
  add_subdirectory (eiskaltdcpp-cli)
endif ()

if (JSONRPC_DAEMON)
add_subdirectory (json)
endif (JSONRPC_DAEMON)

if (HAIKU AND HAIKU_PKG)
  add_subdirectory (haiku)
endif ()

if(GETTEXT_FOUND)
    option (UPDATE_PO "Update po files" OFF)
    option (CREATE_MO "Create mo files" ON)
    add_subdirectory(cmake)
endif(GETTEXT_FOUND)

message(STATUS "CMAKE_SYSTEM: ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
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
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
