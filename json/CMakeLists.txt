project(jsonrpcpp)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(${PROJECT_SOURCE_DIR}/jsonrpc-cpp jsonrpcpp_SRCS)
file (GLOB client_staff ${PROJECT_SOURCE_DIR}/jsonrpc-cpp/*client*)
list(REMOVE_ITEM jsonrpcpp_SRCS ${client_staff})
list(REMOVE_ITEM jsonrpcpp_SRCS ${PROJECT_SOURCE_DIR}/jsonrpc-cpp/jsonrpc_udpserver.cpp
                                ${PROJECT_SOURCE_DIR}/jsonrpc-cpp/jsonrpc_tcpserver.cpp
                                ${PROJECT_SOURCE_DIR}/jsonrpc-cpp/networking.cpp)
set_source_files_properties(${PROJECT_SOURCE_DIR}/jsonrpc-cpp/mongoose.c PROPERTIES COMPILE_FLAGS "-DNDEBUG -DNO_CGI -DNO_SSL")
#option (LOCAL_JSONCPP "Use local JsonCpp" ON)
#if (LOCAL_JSONCPP)
    set (JSONCPP_LIBRARY jsoncpp)
    set (JSONCPP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/jsoncpp/include/)
    include_directories(${JSONCPP_INCLUDE_DIR})
    aux_source_directory(${PROJECT_SOURCE_DIR}/jsoncpp/src jsoncpp_SRCS)
    add_library (jsoncpp STATIC ${jsoncpp_SRCS})
#else (LOCAL_JSONCPP)
    #find_package(JsonCpp)
    #include_directories(${JSONCPP_INCLUDE_DIR})
    #set (JSONCPP ${JSONCPP_LIBRARY})
#endif (LOCAL_JSONCPP)

add_library (jsonrpc-cpp STATIC ${jsonrpcpp_SRCS})
target_link_libraries (jsonrpc-cpp ${JSONCPP_LIBRARY})
