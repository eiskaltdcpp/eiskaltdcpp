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
if (USE_MINIUPNP OR FREE_SPACE_BAR_C)
  set (UPNP "extra")
endif (USE_MINIUPNP OR FREE_SPACE_BAR_C)

add_library (dcpp ${LINK} ${dcpp_srcs})

if (USE_DHT)
  add_definitions ( -DUSE_DHT )
endif (USE_DHT)

if (NOT NATT)
  add_definitions ( -DDISABLE_NAT_TRAVERSAL )
endif (NOT NATT)

if (PERL_REGEX)
  #add_definitions ( -DUSE_PCRE )
  set_property(SOURCE ${PROJECT_SOURCE_DIR}/ADLSearch.cpp PROPERTY COMPILE_DEFINITIONS USE_PCRE )
  set(PCRE ${PCRE_LIBRARY})
endif (PERL_REGEX)

if (eiskaltdcpp_WC_REVISION)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/version.cpp  PROPERTY COMPILE_DEFINITIONS DCPP_REVISION="${eiskaltdcpp_WC_REVISION}")
endif ()

if (LUA_SCRIPT)
    add_definitions ( -DLUA_SCRIPT )
endif (LUA_SCRIPT)

if (USE_MINIUPNP)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/DCPlusPlus.cpp ${PROJECT_SOURCE_DIR}/UPnPManager.cpp  PROPERTY COMPILE_DEFINITIONS USE_MINIUPNP )
endif()

if (HAVE_IFADDRS_H)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/Util.cpp PROPERTY COMPILE_DEFINITIONS HAVE_IFADDRS_H )
endif (HAVE_IFADDRS_H)

if (ICONV_SECOND_ARGUMENT_IS_CONST)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/Text.cpp ${PROJECT_SOURCE_DIR}/UPnPManager.cpp  PROPERTY COMPILE_DEFINITIONS ICONV_CONST="const")
endif (ICONV_SECOND_ARGUMENT_IS_CONST)

if (FORCE_XDG)
   set_property(SOURCE ${PROJECT_SOURCE_DIR}/Util.cpp PROPERTY COMPILE_DEFINITIONS FORCE_XDG )
endif (FORCE_XDG)

if (WIN32)
   set_property(TARGET dcpp PROPERTY COMPILE_FLAGS)
else(WIN32)
   set_property(TARGET dcpp PROPERTY COMPILE_FLAGS -fPIC)
endif (WIN32)

target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${GETTEXT_LIBRARIES} ${ICONV_LIBRARIES} ${WIN32_LIBS} ${APPLE_LIBS} ${LUA_LIBRARIES} ${UPNP} ${PCRE})
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
