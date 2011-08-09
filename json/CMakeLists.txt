project(jsonrpcpp)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include/)

aux_source_directory(${PROJECT_SOURCE_DIR}/src jsonrpcpp_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/lib_json jsoncpp_SRCS)

add_library (jsoncpp STATIC ${jsoncpp_SRCS})
add_library (jsonrpcpp STATIC ${jsonrpcpp_SRCS})

target_link_libraries (jsonrpcpp jsoncpp)
