PROJECT(gnu)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# ######### General setup ##########
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
if (FS_USAGE_C)
add_definitions(-DFS_USAGE_C)
elseif (FS_USAGE_C)
message(STATUS "Sorry but this lib don't build without necessary headers")
endif (FS_USAGE_C)
IF (NOT WIN32)
SET(gnulib_SRCS
    fsusage.c
    )
SET(gnulib_HDRS
    fsusage.h)
# actual target:
ADD_LIBRARY(gnu STATIC ${gnulib_SRCS} ${gnulib_HDRS})
ENDIF (NOT WIN32)
