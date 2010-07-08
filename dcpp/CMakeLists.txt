project (dcpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)

include_directories (${Boost_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${GETTEXT_INCLUDE_DIR})

if (WIN32)
set (LINK STATIC)
set (WIN32_LIBS wsock32)
endif (WIN32)

if (NOT LINK)
set (LINK SHARED)
endif (NOT LINK)

if (APPLE)
set (APPLE_LIBS iconv)
endif (APPLE)

add_library (dcpp ${LINK} ${dcpp_srcs})

target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${GETTEXT_LIBRARIES} ${ICONV_LIBRARIES} ${WIN32_LIBS} ${APPLE_LIBS})

if (APPLE)
set (LIBDIR1 .)
set (LIBDIR ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
endif(APPLE)

if (LINK STREQUAL SHARED)
install (TARGETS dcpp
            LIBRARY DESTINATION ${LIBDIR}
            BUNDLE DESTINATION ${LIBDIR1})
endif (LINK STREQUAL SHARED)
