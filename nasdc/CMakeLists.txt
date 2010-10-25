project(nasdc)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/..)
aux_source_directory(${PROJECT_SOURCE_DIR} nasdc_SRCS)
if (XMLRPC_DAEMON)
include_directories(${XMLRPC_INCLUDE_DIRS})
set (XMLRPCLIBS ${XMLRPC_LIBRARIES})
add_definitions (-DXMLRPC_DAEMON )
endif (XMLRPC_DAEMON)
list (REMOVE_ITEM nasdc_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sampleclient.cpp)
list (REMOVE_ITEM nasdc_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sampleserver.cpp)
# actual target:
add_executable (nasdc MACOSX_BUNDLE WIN32
                ${nasdc_SRCS}
                )
if (XMLRPC_DAEMON)
add_executable (sampleclient sampleclient.cpp)
target_link_libraries (sampleclient ${XMLRPCLIBS})
add_executable (sampleserver sampleserver.cpp)
target_link_libraries (sampleserver ${XMLRPCLIBS})
endif()
target_link_libraries (nasdc dcpp pthread ${XMLRPCLIBS})

#if (UNIX)
 #set (BINDIR bin)
 #if (APPLE)
    #set (BUNDLEDIR .)
 #endif ()
#elseif (WIN32)
  #set (BINDIR .)
#endif (UNIX)
add_definitions(-D_DEBUG)
#set_property (TARGET nasdc PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-daemon")
  #install (TARGETS nasdc
    #RUNTIME DESTINATION ${BINDIR}
    #BUNDLE DESTINATION ${BUNDLEDIR})
