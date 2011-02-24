project(nasdc)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/common ${PROJECT_SOURCE_DIR}/gnu ${PROJECT_SOURCE_DIR}/.. )
aux_source_directory(${PROJECT_SOURCE_DIR} nasdc_SRCS)
#aux_source_directory(${PROJECT_SOURCE_DIR}/common common_SRCS)
#aux_source_directory(${PROJECT_SOURCE_DIR}/gnu gnu_SRCS)
if (XMLRPC_DAEMON)
include_directories(${XMLRPC_INCLUDE_DIRS})
set (XMLRPCLIBS ${XMLRPC_LIBRARIES})
add_definitions (-DXMLRPC_DAEMON )
endif (XMLRPC_DAEMON)

if (CLI_DAEMON)
include_directories(${READLINE_INCLUDE_DIR})
set (RL_LIBS ${READLINE_LIBRARIES})
add_definitions (-DCLI_DAEMON )
endif (CLI_DAEMON)

list (REMOVE_ITEM nasdc_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sampleclient.cpp)
list (REMOVE_ITEM nasdc_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sampleserver.cpp)

if (WIN32)
  list (APPEND nasdc_SRCS ../win32/applink.c)
endif (WIN32)

if (LUA_SCRIPT)
  set_property(SOURCE ${PROJECT_SOURCE_DIR}/ServerThread.cpp PROPERTY COMPILE_DEFINITIONS LUA_SCRIPT)
  include_directories (${LUA_INCLUDE_DIR})
endif (LUA_SCRIPT)
# actual target:
#add_library(nasdc_common STATIC ${common_SRCS})
#add_library(nasdc_gnu STATIC ${gnu_SRCS})

add_executable (nasdc MACOSX_BUNDLE WIN32
                ${nasdc_SRCS}
                )
if (XMLRPC_DAEMON)
add_executable (sampleclient sampleclient.cpp)
target_link_libraries (sampleclient ${XMLRPCLIBS})
#add_executable (sampleserver sampleserver.cpp)
#target_link_libraries (sampleserver ${XMLRPCLIBS})
endif()

target_link_libraries (nasdc dcpp ${XMLRPCLIBS} ${RL_LIBS})

if (UNIX)
 set (BINDIR bin)
 if (APPLE)
    set (BUNDLEDIR .)
 endif ()
elseif (WIN32)
  set (BINDIR .)
endif (UNIX)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
add_definitions(-D_DEBUG)
endif()
set_property (TARGET nasdc PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-daemon")
  install (TARGETS nasdc
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

if (UNIX)
  FIND_PROGRAM (GZIP_CMD gzip)
  EXECUTE_PROCESS (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/eiskaltdcpp-daemon.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-daemon.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

if (APPLE)
  install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-daemon.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
elseif (UNIX AND NOT APPLE)
  install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-daemon.1.gz DESTINATION ${SHARE_DIR}/man/man1)
endif (APPLE)
