PROJECT(fsusage)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# ######### General setup ##########
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
if (FS_USAGE_C)
add_definitions(-DFS_USAGE_C)
aux_source_directory(${PROJECT_SOURCE_DIR} fsusage_SRCS)
if (WIN32)
list (REMOVE_ITEM fsusage_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/fsusage.c)
endif
# actual target:
ADD_LIBRARY(fsusage STATIC ${fsusage_SRCS})

elseif (FS_USAGE_C)
message(STATUS "Sorry but this lib don't build without necessary headers")
endif (FS_USAGE_C)

