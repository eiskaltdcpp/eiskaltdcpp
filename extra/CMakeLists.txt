project(extra)
cmake_minimum_required(VERSION 2.6)
include_directories(${PROJECT_SOURCE_DIR} ${GETTEXT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${MINIUPNP_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR} extra_SRCS)
if (FREE_SPACE_BAR_C)
    if (WIN32)
        list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/fsusage.c)
    endif (WIN32)
else (FREE_SPACE_BAR_C)
    list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/fsusage.c)
    list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/freespace.cpp)
endif(FREE_SPACE_BAR_C)
if (NOT USE_MINIUPNP)
    list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/upnpc.cpp)
    if(NOT MINIUPNP_WITH_IPV6)
        list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/upnpc6.cpp)
    endif()
endif(NOT USE_MINIUPNP)

add_library(extra STATIC ${extra_SRCS})
if (USE_MINIUPNP AND NOT LOCAL_MINIUPNP)
set (CMAKE_REQUIRED_INCLUDES ${MINIUPNP_INCLUDE_DIR})
set (CMAKE_REQUIRED_LIBRARIES ${MINIUPNP_LIBRARY})
check_cxx_source_compiles("
    #include <miniupnpc/miniupnpc.h>
    #include <miniupnpc/upnpcommands.h>
    #include <miniupnpc/upnperrors.h>
    int main()
    {
        UPNPDev *devices = upnpDiscover(5000, 0, 0, 0, 0, 0);
        return 0;
    }" MINIUPNPC16)
    if (MINIUPNPC16)
        add_definitions (-DMINIUPNPC16)
    else()
        list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/upnpc6.cpp)
    endif(MINIUPNPC16)
elseif (USE_MINIUPNP AND LOCAL_MINIUPNP)
    add_definitions (-DMINIUPNPC16)
endif (USE_MINIUPNP AND NOT LOCAL_MINIUPNP)


if (WIN32)
    set_property(TARGET extra PROPERTY COMPILE_FLAGS)
else(WIN32)
    set_property(TARGET extra PROPERTY COMPILE_FLAGS -fPIC)
endif (WIN32)

target_link_libraries (extra ${MINIUPNP_LIBRARY} ${GETTEXT_LIBRARIES})
