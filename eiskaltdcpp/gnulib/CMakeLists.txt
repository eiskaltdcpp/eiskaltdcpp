PROJECT(gnu)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# ######### General setup ##########
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
if (FS_USAGE_C)
add_definitions(-DFS_USAGE_C)
IF (NOT WIN32)
aux_source_directory(${PROJECT_SOURCE_DIR} gnulib_SRCS)
# actual target:
ADD_LIBRARY(gnu STATIC ${gnulib_SRCS})
ENDIF (NOT WIN32)
elseif (FS_USAGE_C)
message(STATUS "Sorry but this lib don't build without necessary headers")
endif (FS_USAGE_C)

