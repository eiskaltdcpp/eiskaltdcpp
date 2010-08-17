PROJECT(miniupnp)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# ######### General setup ##########
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${MINIUPNP_INCLUDE_DIR})

    aux_source_directory(${PROJECT_SOURCE_DIR} miniupnp_SRCS)
# actual target:
    ADD_LIBRARY(miniupnp STATIC ${miniupnp_SRCS})
target_link_libraries (miniupnp ${MINIUPNP_LIBRARY})
