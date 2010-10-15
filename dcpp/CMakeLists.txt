project (dcpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)
file (GLOB dcpp_hdrs ${PROJECT_SOURCE_DIR}/*.h)
set (DCPP_SRCS @dcpp_srcs@ PARENT_SCOPE)
set (DCPP_HDRS @dcpp_hdrs@ PARENT_SCOPE)

if (NOT LUA_SCRIPT)
  list (REMOVE_ITEM dcpp_srcs ${CMAKE_CURRENT_SOURCE_DIR}/ScriptManager.cpp)
endif ()

include_directories (${Boost_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${GETTEXT_INCLUDE_DIR} ${LUA_INCLUDE_DIR})

if (WIN32)
set (LINK STATIC)
set (WIN32_LIBS "ws2_32")
endif (WIN32)

if (NOT LINK)
set (LINK SHARED)
endif (NOT LINK)

if (APPLE)
set (APPLE_LIBS "iconv")
endif (APPLE)

set (UPNP "extra")
add_library (dcpp ${LINK} ${dcpp_srcs})
if (USE_DHT)
add_definitions ( -DUSE_DHT )
endif (USE_DHT)
if(STDIPFILTER)
add_definitions ( -DSTDIPFILTER )
endif(STDIPFILTER)

if (NOT NATT)
add_definitions ( -DDISABLE_NAT_TRAVERSAL )
endif (NOT NATT)
target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${GETTEXT_LIBRARIES} ${ICONV_LIBRARIES} ${WIN32_LIBS} ${APPLE_LIBS} ${LUA_LIBRARIES} ${UPNP})
set_target_properties(dcpp PROPERTIES VERSION ${VERSION} OUTPUT_NAME "eiskaltdcpp")
if (APPLE)
set (LIBDIR1 .)
set (LIBDIR ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
endif(APPLE)

if (LINK STREQUAL SHARED)
install (TARGETS dcpp
            LIBRARY DESTINATION ${LIBDIR} NAMELINK_SKIP
            BUNDLE DESTINATION ${LIBDIR1})
endif (LINK STREQUAL SHARED)
