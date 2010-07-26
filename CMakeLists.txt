project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6)

if (UNIX AND NOT APPLE)
  set (PROJECT_NAME_GLOBAL eiskaltdcpp)
else (UNIX AND NOT APPLE)
  set (PROJECT_NAME_GLOBAL EiskaltDC++)
endif (UNIX AND NOT APPLE)

set (VERSION "2.0.x")
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

find_package (PkgConfig)
find_package (BZip2 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (Threads REQUIRED)
find_package (Gettext REQUIRED)
find_package (Iconv REQUIRED)
find_package (Subversion)

option (USE_QT "Qt interface" ON)
option (USE_WT "WebControl" OFF)
option (USE_GTK "GTK interface" OFF)


if (USE_QT)
    option (USE_LIBUPNP "Libupnp" ON)
else (USE_QT)
    option (USE_LIBUPNP "Libupnp" OFF)
endif (USE_QT)

if (USE_LIBUPNP)
    message (STATUS "Building with UPNP support")
    find_package (LibUpnp REQUIRED)
else (USE_LIBUPNP)
    message (STATUS "Building without UPNP support")
endif (USE_LIBUPNP)

if (USE_QT)
    find_package (Qt4 4.4.0 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED)
    option (DBUS_NOTIFY "QtDbus support" ON)
    if (DBUS_NOTIFY)
        find_package (Qt4 4.4.0 COMPONENTS QtDBus REQUIRED)
    endif (DBUS_NOTIFY)
    option (USE_JS "QtScript support")
    if (USE_JS)
        find_package (Qt4 4.4.0 COMPONENTS QtScript REQUIRED)
    endif (USE_JS)
endif (USE_QT)


option(USE_ASPELL "Use aspell for spellchecking")
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

option (FORCE_XDG "Respect freedesktop.org standards" ON)

option (FREE_SPACE_BAR_C "Use for show free space on volume small c lib" ON)
if (NOT USE_QT)
    set (FREE_SPACE_BAR_C OFF)
endif (NOT USE_QT)

if (FREE_SPACE_BAR_C)
    set (FREE_SPACE_BAR_C 1 CACHE BOOL "Use for show free space on volume small c lib" FORCE)
    if (NOT LOCAL_BOOST)
        find_package (Boost REQUIRED)
    endif (NOT LOCAL_BOOST)
    message (STATUS "Building with free space bar (realisation on C, POSIX 1003.1-2001 (and later) with XSI)")
endif (FREE_SPACE_BAR_C)

if (NOT FREE_SPACE_BAR_C)
    set (FREE_SPACE_BAR_C 0 CACHE BOOL "Use for show free space on volume small c lib" FORCE)
    if (NOT LOCAL_BOOST)
        find_package (Boost REQUIRED)
    endif (NOT LOCAL_BOOST)
    message (STATUS "Building without free space bar")
endif (NOT FREE_SPACE_BAR_C)

if (USE_WT)
    find_package (Wt REQUIRED)
endif (USE_WT)

if (USE_GTK)
    find_package (GTK2 REQUIRED gtk glade)

    option (USE_LIBGNOME2 "LibGnome2" ON) # not available now
    if (USE_LIBGNOME2)
        message (STATUS "Building with LibGnome2 support")
        find_package (LibGnome2 REQUIRED)
    else (USE_LIBGNOME2)
        message (STATUS "Building without LibGnome2 support")
    endif (USE_LIBGNOME2)
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
endif(LOCAL_BOOST)

if (NOT CMAKE_INSTALL_PREFIX)
  set (CMAKE_INSTALL_PREFIX "/usr")
endif (NOT CMAKE_INSTALL_PREFIX)

if (LIB_INSTALL_DIR)
  set (LIBDIR ${LIB_INSTALL_DIR})
endif (LIB_INSTALL_DIR)

if (NOT LIBDIR)
  set (LIBDIR "lib")
  message (STATUS "Library directory: ${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
endif (NOT LIBDIR)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)

option (WITH_EXAMPLES "Install examples sh and others scripts" ON)

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

set (_DATADIR "{CLIENT_DATA_DIR}")

message (STATUS "Data directory: ${CLIENT_DATA_DIR}")

if (Subversion_FOUND)
    Subversion_WC_INFO (${PROJECT_SOURCE_DIR} eiskaltdcpp)
    message (STATUS "Build svn revision: ${eiskaltdcpp_WC_REVISION}")
    if (eiskaltdcpp_WC_REVISION MATCHES [0-9]+)
      add_definitions (-DDCPP_REVISION="\(r""${eiskaltdcpp_WC_REVISION}""\)")
    endif ()
endif (Subversion_FOUND)

if (UNIX)
add_definitions (-D_GNU_SOURCE)
endif (UNIX)

set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/ts")
set (CLIENT_SOUNDS_DEST "${CLIENT_DATA_DIR}/sounds")
set (CLIENT_ICONS_DEST "${CLIENT_DATA_DIR}/icons")
set (CLIENT_SCRIPTS_DEST "${CLIENT_DATA_DIR}/scripts")

add_definitions (-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT
                 -D_DATADIR="${_DATADIR}" -DBUILDING_DCPP)

if (HAVE_IFADDRS_H)
  add_definitions (-DHAVE_IFADDRS_H )
endif (HAVE_IFADDRS_H)

if (ICONV_SECOND_ARGUMENT_IS_CONST)
  add_definitions (-DICONV_CONST=const)
endif (ICONV_SECOND_ARGUMENT_IS_CONST)

if (FORCE_XDG)
  add_definitions (-DFORCE_XDG)
endif (FORCE_XDG)

if (USE_JS)
  add_definitions (-DUSE_JS)
endif (USE_JS)

if (USE_ICON_THEME)
 add_definitions(-DUSE_ICON_THEME )
endif (USE_ICON_THEME)

set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pipe -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE     "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG -pipe")
set (CMAKE_CXX_FLAGS_DEBUG       "-UNDEBUG -g3 -pipe")

include_directories(${eiskaltdcpp_BINARY_DIR}
   ${eiskaltdcpp_BINARY_DIR}/dcpp
   ${eiskaltdcpp_SOURCE_DIR}/dcpp
   ${eiskaltdcpp_BINARY_DIR}/eiskaltdcpp-qt
   ${eiskaltdcpp_SOURCE_DIR}/eiskaltdcpp-qt
   ${eiskaltdcpp_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   )

message(STATUS "CMAKE will use this libs(and headers) during build:

                upnp (headers): ${UPNP_LIBRARY} (${UPNP_INCLUDE_DIR})
                boost (headers): ${Boost_LIBRARIES} (${Boost_INCLUDE_DIR})
                bzip2 (headers): ${BZIP2_LIBRARIES} (${BZIP2_INCLUDE_DIR})
                zlib (headers): ${ZLIB_LIBRARIES} (${ZLIB_INCLUDE_DIR})
                openssl (headers): ${OPENSSL_LIBRARIES} (${OPENSSL_INCLUDE_DIR})
                iconv (headers): ${ICONV_LIBRARIES} (${ICONV_INCLUDE_DIR})
                aspell (headers): ${ASPELL_LIBRARIES} (${ASPELL_INCLUDE_DIR})
                gettext (headers): ${GETTEXT_LIBRARIES} (${GETTEXT_INCLUDE_DIR})
                ")
#add_subdirectory (dht)
add_subdirectory (dcpp)

if (USE_QT)
  add_subdirectory (eiskaltdcpp-qt)
endif (USE_QT)

if (USE_WT)
  add_subdirectory (eiskaltdcpp-wt)
endif (USE_WT)

if (USE_GTK)
  add_subdirectory (eiskaltdcpp-gtk)
endif (USE_GTK)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

