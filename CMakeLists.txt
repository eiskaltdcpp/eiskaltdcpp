project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6)
set (PROJECT_NAME_GLOBAL eiskaltdcpp)

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

if (ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
  # Already in cache, be silent
  set(ICONV_FIND_QUIETLY TRUE)
endif (ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)

find_path(ICONV_INCLUDE_DIR iconv.h)

find_library(ICONV_LIBRARIES NAMES iconv libiconv libiconv-2 c)

if (ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
   set (ICONV_FOUND TRUE)
endif (ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)

set(CMAKE_REQUIRED_INCLUDES ${ICONV_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})

if (ICONV_FOUND)
  check_cxx_source_compiles("
  #include <iconv.h>
  int main(){
    iconv_t conv = 0;
    const char* in = 0;
    size_t ilen = 0;
    char* out = 0;
    size_t olen = 0;
    iconv(conv, &in, &ilen, &out, &olen);
    return 0;
  }
" ICONV_SECOND_ARGUMENT_IS_CONST )
endif (ICONV_FOUND)

set (CMAKE_REQUIRED_INCLUDES)
set (CMAKE_REQUIRED_LIBRARIES)

if (ICONV_FOUND)
  if (NOT ICONV_FIND_QUIETLY)
    message (STATUS "Found Iconv: ${ICONV_LIBRARIES}")
  endif (NOT ICONV_FIND_QUIETLY)
else (ICONV_FOUND)
  if (Iconv_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find Iconv")
  endif (Iconv_FIND_REQUIRED)
endif (ICONV_FOUND)

MARK_AS_ADVANCED(
  ICONV_INCLUDE_DIR
  ICONV_LIBRARIES
  ICONV_SECOND_ARGUMENT_IS_CONST
)

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

find_package (BZip2 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (Threads REQUIRED)
find_package (Gettext REQUIRED)
find_package (Qt4 4.4 COMPONENTS QtCore QtGui QtXml QtNetwork QtDBus REQUIRED )
find_package (PkgConfig)

if (NOT USE_ASPELL)
    option(USE_ASPELL "Use aspell for spellchecking")
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

#if (NOT (LOCAL_BOOST EQUAL 0))
#set (LOCAL_BOOST 1)
#endif (NOT (LOCAL_BOOST EQUAL 0))
option (FREE_SPACE_BAR "Use for show free space on volume boost libs")
option (FREE_SPACE_BAR_C "Use for show free space on volume small c lib")
if (FREE_SPACE_BAR AND NOT FREE_SPACE_BAR_C)
if (NOT LOCAL_BOOST)
find_package (Boost COMPONENTS date_time filesystem system REQUIRED)
message (STATUS "Build with free space bar (boost)")
else (NOT LOCAL_BOOST)
message (STATUS "Sorry, you add flag when use boost libs with flag when change ")
message (STATUS "local boost headers, may build this config only with free space bar (c realized)")
set (FREE_SPACE_BAR_C 1 CACHE BOOL "Use for show free space on volume small c lib" FORCE)
set (FREE_SPACE_BAR 0 CACHE BOOL "Use for show free space on volume boost libs" FORCE)
endif (NOT LOCAL_BOOST)
endif (FREE_SPACE_BAR AND NOT FREE_SPACE_BAR_C)

if (FREE_SPACE_BAR_C AND NOT FREE_SPACE_BAR)
set (FREE_SPACE_BAR_C 1 CACHE BOOL "Use for show free space on volume small c lib" FORCE)
if (NOT LOCAL_BOOST)
find_package (Boost REQUIRED)
endif (NOT LOCAL_BOOST)
message (STATUS "Build with free space bar (realisation on C, POSIX 1003.1-2001 (and later) with XSI)")
endif (FREE_SPACE_BAR_C AND NOT FREE_SPACE_BAR)

if (FREE_SPACE_BAR AND FREE_SPACE_BAR_C)
set (FREE_SPACE_BAR 0 CACHE BOOL "Use for show free space on volume boost libs" FORCE)
set (FREE_SPACE_BAR_C 1 CACHE BOOL "Use for show free space on volume small c lib" FORCE)
if (NOT LOCAL_BOOST)
find_package (Boost REQUIRED)
endif (NOT LOCAL_BOOST)
message (STATUS "Build with free space bar (realisation on C, POSIX 1003.1-2001 (and later) with XSI)")
endif (FREE_SPACE_BAR AND FREE_SPACE_BAR_C)

if (NOT FREE_SPACE_BAR AND NOT FREE_SPACE_BAR_C)
set (FREE_SPACE_BAR 0 CACHE BOOL "Use for show free space on volume boost libs" FORCE)
set (FREE_SPACE_BAR_C 0 CACHE BOOL "Use for show free space on volume small c lib" FORCE)
if (NOT LOCAL_BOOST)
find_package (Boost REQUIRED)
endif (NOT LOCAL_BOOST)
message (STATUS "Build without free space bar")
endif (NOT FREE_SPACE_BAR AND NOT FREE_SPACE_BAR_C)

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
set (Boost_INCLUDEDIR "./")
message (STATUS "USED LOCAL BOOST HEADERS")
endif(LOCAL_BOOST)

pkg_check_modules (PC_UPNP REQUIRED libupnp)

set (UPNP_DEFINITIONS ${PC_UPNP_CFLAGS_OTHER})

find_path(UPNP_INCLUDE_DIR upnp/upnp.h
   HINTS
   ${PC_UPNP_INCLUDEDIR}
   ${PC_UPNP_INCLUDE_DIRS}
   PATH_SUFFIXES upnp)
find_library(UPNP_UPNP_LIBRARY upnp
   HINTS
   ${PC_UPNP_LIBDIR}
   ${PC_UPNP_LIBRARY_DIRS})
find_library(UPNP_IXML_LIBRARY ixml
   HINTS
   ${PC_UPNP_LIBDIR}
   ${PC_UPNP_LIBRARY_DIRS})
find_library(UPNP_THREADUTIL_LIBRARY threadutil
   HINTS
   ${PC_UPNP_LIBDIR}
   ${PC_UPNP_LIBRARY_DIRS})

if (UPNP_INCLUDE_DIR AND UPNP_UPNP_LIBRARY)
    set (UPNP_FOUND 1)
endif (UPNP_INCLUDE_DIR AND UPNP_UPNP_LIBRARY)

if (UPNP_FOUND)
    set (UPNP_LIBRARY ${UPNP_UPNP_LIBRARY} ${UPNP_IXML_LIBRARY} ${UPNP_THREADUTIL_LIBRARY} ${PC_UPNP_LDFLAGS})
    message (STATUS "Found the upnp libraries at ${UPNP_LIBRARY}")
    message (STATUS "Found the upnp headers at ${UPNP_INCLUDE_DIR}")
else (UPNP_FOUND)
    message (STATUS "Could not find upnp")
endif (UPNP_FOUND)

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

if (UNIX AND NOT APPLE)
  set (DATAPATH "share/eiskaltdcpp")
elseif (APPLE)
  set (DATAPATH "./")
endif (UNIX AND NOT APPLE)

set (SHARE_DIR "share")

if (APPLE)
  set (CLIENT_DATA_DIR "${PROJECT_NAME_GLOBAL}.app")
else (APPLE)
  set (CLIENT_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/${PROJECT_NAME_GLOBAL}")
endif (APPLE)

set (_DATADIR "${CLIENT_DATA_DIR}")

message (STATUS "Data directory: ${CLIENT_DATA_DIR}")

if (NOT DEFINED SVNVERSION)
        find_program (SVNVERSION_CMD svnversion)
        execute_process (COMMAND "${SVNVERSION_CMD}"
                OUTPUT_VARIABLE SVNVERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        message (STATUS "Build svn revision: ${SVNVERSION}")
endif (NOT DEFINED SVNVERSION)
if (NOT SVNVERSION OR SVNVERSION STREQUAL exported)
else (SVNVERSION)
        add_definitions(-DDCPP_REVISION="\(r""${SVNVERSION}""\)")
endif (NOT SVNVERSION OR SVNVERSION STREQUAL exported)
if (UNIX)
add_definitions (-D_GNU_SOURCE -D_LARGEFILE_SOURCE)
endif (UNIX)

set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/ts")
set (CLIENT_SOUNDS_DEST "${CLIENT_DATA_DIR}/sounds")
set (CLIENT_ICONS_DEST "${CLIENT_DATA_DIR}/icons")

add_definitions (-pipe -O2 -I. -fno-builtin -D_FILE_OFFSET_BITS=64
                 -D_REENTRANT -D_DATADIR="${_DATADIR}"
                 -DNDEBUG -DBUILDING_DCPP)

if (HAVE_IFADDRS_H)
  add_definitions (-DHAVE_IFADDRS_H -DHAVE_IF_ADDRS_H)
endif (HAVE_IFADDRS_H)

if (ICONV_SECOND_ARGUMENT_IS_CONST)
  add_definitions (-DICONV_CONST=const)
endif (ICONV_SECOND_ARGUMENT_IS_CONST)

set(CMAKE_CXX_FLAGS_RELEASE     "-DNDEBUG -DQT_NO_DEBUG -O3 -fomit-frame-pointer -pipe")
set(CMAKE_CXX_FLAGS_DEBUG       "-UNDEBUG -g3 -pipe")

include_directories(${eiskaltdcpp_BINARY_DIR}
   ${eiskaltdcpp_BINARY_DIR}/dcpp
   ${eiskaltdcpp_SOURCE_DIR}/dcpp
   ${eiskaltdcpp_BINARY_DIR}/eiskaltdcpp
   ${eiskaltdcpp_SOURCE_DIR}/eiskaltdcpp
   ${eiskaltdcpp_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   )
message(STATUS "CMAKE use in build this libs(and headers):

                upnp (headers): ${UPNP_LIBRARY} (${UPNP_INCLUDE_DIR})
                boost (headers): ${Boost_LIBRARIES} (${Boost_INCLUDE_DIR})
                bzip2 (headers): ${BZIP2_LIBRARIES} (${BZIP2_INCLUDE_DIR})
                zlib (headers): ${ZLIB_LIBRARIES} (${ZLIB_INCLUDE_DIR})
                openssl (headers): ${OPENSSL_LIBRARIES} (${OPENSSL_INCLUDE_DIR})
                iconv (headers): ${ICONV_LIBRARIES} (${ICONV_INCLUDE_DIR})
                aspell (headers): ${ASPELL_LIBRARIES} (${ASPELL_INCLUDE_DIR})
                ")

add_subdirectory (dcpp)
add_subdirectory (eiskaltdcpp)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

