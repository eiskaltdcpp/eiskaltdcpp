PROJECT(fsusage)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# ######### General setup ##########
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

    aux_source_directory(${PROJECT_SOURCE_DIR} fsusage_SRCS)
    if (WIN32)
        list (REMOVE_ITEM fsusage_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/fsusage.c)
    endif (WIN32)
# actual target:
    ADD_LIBRARY(fsusage STATIC ${fsusage_SRCS})

