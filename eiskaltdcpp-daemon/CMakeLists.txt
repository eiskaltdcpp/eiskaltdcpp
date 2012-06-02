project(${PROJECT_NAME_GLOBAL}-daemon)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/..)
aux_source_directory(${PROJECT_SOURCE_DIR} nasdc_SRCS)

if (JSONRPC_DAEMON AND XMLRPC_DAEMON)
    message("   Build both JSONRPC and XMLRPC interfaces is prohibited for now, set XMLRPC_DAEMON=OFF")
    set (XMLRPC_DAEMON OFF CACHE BOOL "Make daemon as xmlrpc server" FORCE)
endif (JSONRPC_DAEMON AND XMLRPC_DAEMON)

if (JSONRPC_DAEMON)
    message (STATUS "Build daemon with JSONRPC interface")
    include_directories(${PROJECT_SOURCE_DIR}/../json/jsoncpp/)
    set(JSONRPCLIBS jsonrpc-cpp)
    add_definitions (-DJSONRPC_DAEMON)
else (JSONRPC_DAEMON)
    list (REMOVE_ITEM nasdc_SRCS ${PROJECT_SOURCE_DIR}/jsonrpcmethods.cpp)
endif (JSONRPC_DAEMON)

if (XMLRPC_DAEMON)
    message (STATUS "Build daemon with XMLRPC interface")
    if (NOT NO_UI_DAEMON)
        set (NO_UI_DAEMON 1 CACHE BOOL "Daemon without anyone ui" FORCE)
    endif (NOT NO_UI_DAEMON)
    set (xmlrpclibs "abyss-server")
    find_package (XMLRPC REQUIRED c++2 ${xmlrpclibs})
    include_directories(${XMLRPC_INCLUDE_DIRS})
    set (XMLRPCLIBS ${XMLRPC_LIBRARIES})
    add_definitions (-DXMLRPC_DAEMON )
endif (XMLRPC_DAEMON)

if (WITH_DHT)
  add_definitions ( -DWITH_DHT )
endif (WITH_DHT)

if (HAVE_DAEMON)
    add_definitions(-DHAVE_DAEMON)
endif(HAVE_DAEMON)

if (WIN32)
  list (APPEND nasdc_SRCS ../win32/applink.c)
endif (WIN32)

if (LUA_SCRIPT)
  include_directories (${LUA_INCLUDE_DIR})
endif (LUA_SCRIPT)

if (ENABLE_STACKTRACE)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/nasdc.cpp PROPERTY COMPILE_DEFINITIONS ENABLE_STACKTRACE)
endif (ENABLE_STACKTRACE)

# actual target:
add_executable (${PROJECT_NAME} MACOSX_BUNDLE WIN32
                ${nasdc_SRCS}
                )

target_link_libraries (${PROJECT_NAME} dcpp ${XMLRPCLIBS} ${JSONRPCLIBS})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-D_DEBUG)
endif()

install (TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

if (UNIX)
  execute_process (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE ${PROJECT_NAME}.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
  install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz DESTINATION ${MAN_DIR})
endif (UNIX)
