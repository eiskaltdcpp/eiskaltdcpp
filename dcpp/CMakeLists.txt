project (dcpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)

include_directories (${Boost_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${GETTEXT_INCLUDE_DIR})

if(WIN32)
add_library (dcpp STATIC ${dcpp_srcs})
else()
add_library (dcpp SHARED ${dcpp_srcs})
endif()

if (APPLE)
target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${ICONV_LIBRARIES} ${GETTEXT_LIBRARIES} iconv)
elseif (WIN32)
target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${ICONV_LIBRARIES} ${GETTEXT_LIBRARIES} -lwsock32)
else ()
target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${ICONV_LIBRARIES} ${GETTEXT_LIBRARIES})
endif ()

if (APPLE)
set (LIBDIR1 .)
set (LIBDIR ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
endif(APPLE)

if (UNIX)
install (TARGETS dcpp
            LIBRARY DESTINATION ${LIBDIR}
            BUNDLE DESTINATION ${LIBDIR1})
endif (UNIX)
