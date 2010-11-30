project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6)

if (UNIX AND NOT APPLE)
  set (PROJECT_NAME_GLOBAL eiskaltdcpp)
else (UNIX AND NOT APPLE)
  set (PROJECT_NAME_GLOBAL EiskaltDC++)
endif (UNIX AND NOT APPLE)

set (VERSION "2.1.x")
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

find_package (X11)
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
option (USE_GTK "GTK interface" OFF)
option (DHT "DHT support for dcpp" OFF)
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
option (DBUS_NOTIFY "QtDbus support in Qt interface" ON)
option (USE_JS "QtScript support in Qt interface")
option (NATT "Use natt extension" ON)
option (NO_UI_DAEMON "Daemon without anyone ui" OFF)
option (XMLRPC_DAEMON "Mаке no ui daemon as xmlrpc server" OFF)
option (PERL_REGEX "Use pcre lib for regex in perl style" OFF)

if (PERL_REGEX)
find_package(Pcre REQUIRED)
endif (PERL_REGEX)

if (LUA_SCRIPT)
    find_package (Lua51 REQUIRED)
    #add_definitions ( -DLUA_SCRIPT )
    message (STATUS "Building with Lua scripts support")
else (LUA_SCRIPT)
    message (STATUS "Building without Lua scripts support")
endif(LUA_SCRIPT)

if (USE_MINIUPNP)
    message (STATUS "Building with UPNP support (miniupnpc)")

  if (LOCAL_MINIUPNP)
    add_subdirectory(miniupnpc)
    set (MINIUPNP_INCLUDE_DIR "./")
    set (MINIUPNP_LIBRARY "miniupnpc")
  else (LOCAL_MINIUPNP)
    find_package (Miniupnpc REQUIRED)
  endif (LOCAL_MINIUPNP)

elseif (NOT USE_MINIUPNP)
    unset (MINIUPNP_INCLUDE_DIR CACHE)
    unset (MINIUPNP_LIBRARY CACHE)
    message (STATUS "Building without UPNP support (miniupnpc)")
endif(USE_MINIUPNP)

if (XMLRPC_DAEMON)
    if (NOT NO_UI_DAEMON)
        set (NO_UI_DAEMON 1 CACHE BOOL "Daemon without anyone ui" FORCE)
    endif (NOT NO_UI_DAEMON)
    find_package (XMLRPC REQUIRED c++2 abyss-server client)
endif (XMLRPC_DAEMON)

if (USE_QT)
    if (USE_QT_QML)
      find_package (Qt4 4.7.0 COMPONENTS QtCore QtGui QtXml QtNetwork QtDeclarative REQUIRED)
      message (STATUS "Building with Qt Declarative Ui support")
      set (USE_JS ON)
    else (USE_QT_QML)
      find_package (Qt4 4.4.0 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED)
    endif (USE_QT_QML)

    if (DBUS_NOTIFY)
        find_package (Qt4 4.4.0 COMPONENTS QtDBus REQUIRED)
    endif (DBUS_NOTIFY)
    if (USE_JS)
        find_package (Qt4 4.4.0 COMPONENTS QtScript REQUIRED)
	message (STATUS "Building with Qt Script support")
    endif (USE_JS)
endif (USE_QT)


if (NOT USE_QT)
    set (USE_ASPELL OFF)
    set (FREE_SPACE_BAR_C OFF)
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

if (USE_GTK)

    find_package (GTK2 REQUIRED gtk glade)

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
endif (USE_GTK)

if (LOCAL_BOOST)
file(GLOB BOOST_TAR_EXISTS  *.tar.gz)
if (NOT BOOST_TAR_EXISTS)
set (boost_file_name "boost.tar.gz")
file(DOWNLOAD http://eiskaltdc.googlecode.com/files/${boost_file_name} ${CMAKE_CURRENT_SOURCE_DIR}/${boost_file_name} LOG boost_logs STATUS boost_status)
message (STATUS "${boost_logs}")
message (STATUS "${boost_status}")
endif (NOT BOOST_TAR_EXISTS)
file(GLOB BOOST_TAR_EXISTS  *.tar.gz)
FIND_PROGRAM (TAR_CMD tar)
  EXECUTE_PROCESS (
                COMMAND ${TAR_CMD} -xf ${BOOST_TAR_EXISTS}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
set (Boost_INCLUDE_DIR "./")
message (STATUS "USED LOCAL BOOST HEADERS")
else (LOCAL_BOOST)
    find_package (Boost REQUIRED)
    if (Boost_VERSION VERSION_LESS 103900)
    add_definitions(-DFIX_FOR_OLD_BOOST)
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

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)

if (NOT SHARE_DIR)
set (SHARE_DIR "share")
endif (NOT SHARE_DIR)

if (APPLE)
  set (CLIENT_DATA_DIR "${PROJECT_NAME_GLOBAL}.app")
elseif (WIN32)
  set (CLIENT_DATA_DIR "${SHARE_DIR}")
else (APPLE)
  set (CLIENT_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/${PROJECT_NAME_GLOBAL}")
endif (APPLE)

if (APPLE)
  set (LOCALE_DIR "${PROJECT_NAME_GLOBAL}.app/locale")
elseif (WIN32)
  set (LOCALE_DIR "${SHARE_DIR}/locale")
else (APPLE)
  set (LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/locale")
endif (APPLE)

set (_DATADIR "${CLIENT_DATA_DIR}") # don't change! (used by eiskaltdcpp-gtk)

message (STATUS "Data directory: ${CLIENT_DATA_DIR}")

if (DCPP_REVISION)
    set (eiskaltdcpp_WC_REVISION ${DCPP_REVISION})
    message (STATUS "DCPP_REVISION is set to: ${eiskaltdcpp_WC_REVISION}")
    #add_definitions (-DDCPP_REVISION="${eiskaltdcpp_WC_REVISION}")
elseif (GIT_FOUND)
    execute_process (COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                OUTPUT_VARIABLE eiskaltdcpp_WC_REVISION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
    message (STATUS "Build Git commit ID: ${eiskaltdcpp_WC_REVISION}")
    #if (eiskaltdcpp_WC_REVISION MATCHES [0-9a-z]+)
        #add_definitions (-DDCPP_REVISION="${eiskaltdcpp_WC_REVISION}")
    #endif ()
elseif (Subversion_FOUND)
    Subversion_WC_INFO (${PROJECT_SOURCE_DIR} eiskaltdcpp)
    message (STATUS "Build SVN revision: ${eiskaltdcpp_WC_REVISION}")
    if (eiskaltdcpp_WC_REVISION MATCHES [0-9]+)
        #add_definitions (-DDCPP_REVISION="r${eiskaltdcpp_WC_REVISION}")
        set (eiskaltdcpp_WC_REVISION "r${eiskaltdcpp_WC_REVISION}")
    endif ()
endif ()

if (UNIX)
add_definitions (-D_GNU_SOURCE)
endif (UNIX)

set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/qt/ts")
set (CLIENT_SOUNDS_DEST "${CLIENT_DATA_DIR}/sounds")
set (CLIENT_ICONS_DEST "${CLIENT_DATA_DIR}/icons")
set (CLIENT_SCRIPTS_DEST "${CLIENT_DATA_DIR}/qt/qtscripts")

add_definitions (-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT
                 -D_DATADIR="${_DATADIR}" -DLOCALE_DIR="${LOCALE_DIR}" -DBUILDING_DCPP)

#if (HAVE_IFADDRS_H)
  #add_definitions (-DHAVE_IFADDRS_H )
#endif (HAVE_IFADDRS_H)

#if (ICONV_SECOND_ARGUMENT_IS_CONST)
  #add_definitions (-DICONV_CONST=const)
#endif (ICONV_SECOND_ARGUMENT_IS_CONST)

#if (FORCE_XDG)
  #add_definitions (-DFORCE_XDG)
#endif (FORCE_XDG)

if (CMAKE_COMPILER_IS_GNUСXX)
    if (CMAKE_CROSSCOMPILING)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -enable-auto-import")
    endif (CMAKE_CROSSCOMPILING)
    if (NOT APPLE)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--as-needed")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed")
    endif (NOT APPLE)
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pipe -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE     "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG -pipe")
    set (CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -UNDEBUG -g3 -pipe")
endif (CMAKE_COMPILER_IS_GNUСXX)

include_directories(${eiskaltdcpp_BINARY_DIR}
   ${eiskaltdcpp_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   )

message(STATUS "CMAKE_SYSTEM: ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_COMPILER_IS_GNUСXX: ${CMAKE_COMPILER_IS_GNUCXX}")
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
                pcre(cpp) (headers): ${PCRE_LIBRARY} (${PCRE_INCLUDE_DIR})

                ")

if (UNIX AND NOT APPLE)
  set (CLIENT_ICONS_DEST1 ${SHARE_DIR}/icons)
  set (CLIENT_ICONS_DEST2 ${SHARE_DIR}/${PROJECT_NAME_GLOBAL}/icons)
else ()
  set (CLIENT_ICONS_DEST1 ${CLIENT_ICONS_DEST})
  set (CLIENT_ICONS_DEST2 ${CLIENT_ICONS_DEST})
endif ()

option (LANGUAGES "Using translations")
list (APPEND AllLanguages
            be
            bg
            en
            es
            fr
            hu
            pl
            ru
            sr
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

if (DHT)
add_subdirectory (dht)
endif (DHT)

add_subdirectory (dcpp)

if (USE_QT)
  add_subdirectory (eiskaltdcpp-qt)
endif (USE_QT)

if (USE_GTK)
  add_subdirectory (eiskaltdcpp-gtk)
endif (USE_GTK)

if (NO_UI_DAEMON)
add_subdirectory (nasdc)
endif (NO_UI_DAEMON)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

if(GETTEXT_FOUND)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/KWWidgetsInternationalizationMacros.cmake)
option (UPDATE_PO "Update po files" OFF)
option (CREATE_MO "Create mo files" ON)

if (UPDATE_PO)
set (create_pot 1)
set (create_po 1)
set (create_po_lo 1)
else (UPDATE_PO)
set (create_pot 0)
set (create_po 0)
set (create_po_lo 0)
endif (UPDATE_PO)
if (CREATE_MO)
set (create_mo 1)
set (create_mo_lo 1)
else (CREATE_MO)
set (create_mo 0)
set (create_mo_lo 0)
endif (CREATE_MO)
set (mo_for_all 1)
    kwwidgets_create_gettext_targets(
      DOMAIN_NAME "libeiskaltdcpp"
      LOCALE_LIST "${LANGUAGES}"
      COPYRIGHT_HOLDER "EiskaltDC++ team"
      SOURCES "${DCPP_SRCS};${DCPP_HDRS}"
      PO_DIR "${dcpp_SOURCE_DIR}/po"
      POT_BUILD_DIR "${dcpp_SOURCE_DIR}/po"
      #PO_PREFIX    ""
      PO_BUILD_DIR "${dcpp_SOURCE_DIR}/po"
      MO_BUILD_DIR "${dcpp_BINARY_DIR}/locale"
      MO_INSTALL_DIR "/${LOCALE_DIR}"
      CREATE_POT_TARGET ${create_pot}
      CREATE_PO_TARGET ${create_po}
      CREATE_PO_LOCALE_TARGETS ${create_po_lo}
      CREATE_MO_TARGET ${create_mo}
      CREATE_MO_LOCALE_TARGETS ${create_mo_lo}
      ADD_MO_TARGET_TO_ALL ${mo_for_all}
      )
if (USE_GTK)
     kwwidgets_create_gettext_targets(
      DOMAIN_NAME "eiskaltdcpp-gtk"
      LOCALE_LIST "${LANGUAGES}"
      COPYRIGHT_HOLDER "EiskaltDC++ team"
      SOURCES "${GTK_SRCS};${GTK_HDRS};${GTK_GLADE}"
      PO_DIR "${${PROJECT_NAME_GLOBAL}-gtk_SOURCE_DIR}/po"
      POT_BUILD_DIR "${${PROJECT_NAME_GLOBAL}-gtk_SOURCE_DIR}/po"
      #PO_PREFIX    ""
      PO_BUILD_DIR "${${PROJECT_NAME_GLOBAL}-gtk_SOURCE_DIR}/po"
      MO_BUILD_DIR "${${PROJECT_NAME_GLOBAL}-gtk_BINARY_DIR}/locale"
      MO_INSTALL_DIR "/${LOCALE_DIR}"
      CREATE_POT_TARGET ${create_pot}
      CREATE_PO_TARGET ${create_po}
      CREATE_PO_LOCALE_TARGETS ${create_po_lo}
      CREATE_MO_TARGET ${create_mo}
      CREATE_MO_LOCALE_TARGETS ${create_mo_lo}
      ADD_MO_TARGET_TO_ALL ${mo_for_all}
      )

endif (USE_GTK)

endif(GETTEXT_FOUND)

if (USE_QT OR USE_GTK)

    if (WITH_EXAMPLES)
        install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        PATTERN ".svn" EXCLUDE )
    endif (WITH_EXAMPLES)

    if (WITH_LUASCRIPTS AND LUA_SCRIPT)
        install (DIRECTORY luascripts/ DESTINATION ${CLIENT_DATA_DIR}/luascripts
        PATTERN ".svn" EXCLUDE )
    endif (WITH_LUASCRIPTS AND LUA_SCRIPT)

    if (WITH_SOUNDS)
        install (DIRECTORY sounds/ DESTINATION ${CLIENT_DATA_DIR}/sounds
        PATTERN ".svn" EXCLUDE )
    endif (WITH_SOUNDS)

    if (WITH_EMOTICONS)
        install (DIRECTORY emoticons/ DESTINATION ${CLIENT_DATA_DIR}/emoticons
        PATTERN ".svn" EXCLUDE)
    endif (WITH_EMOTICONS)
list(APPEND iconsize 16 22 24 32 48 64 128)
foreach (isize ${iconsize})
  install (FILES icons/icon_${isize}x${isize}.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/${isize}x${isize}/apps RENAME eiskaltdcpp.png)
endforeach (isize)
if (APPLE)
  install (FILES icons/icon_128x128.png DESTINATION ${PROJECT_NAME_GLOBAL}.app/pixmaps/ RENAME eiskaltdcpp.png)
else (APPLE)
  install (FILES icons/icon_128x128.png DESTINATION ${SHARE_DIR}/pixmaps/ RENAME eiskaltdcpp.png)
endif (APPLE)
endif (USE_QT OR USE_GTK)
