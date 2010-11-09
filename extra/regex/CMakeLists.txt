project(pcre)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR} pcre_SRCS)
add_library(pcre STATIC ${pcre_SRCS})

