project (dcpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)

include_directories (${Boost_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${GETTEXT_INCLUDE_DIR})

add_library (dcpp SHARED ${dcpp_srcs})

target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${ICONV_LIBRARIES} ${GETTEXT_LIBRARIES} ${GETTEXT_INTL_LIBRARY}) #iconv intl)

if (NOT APPLE)
install (TARGETS dcpp
            LIBRARY DESTINATION ${LIBDIR})
else (NOT APPLE)
install (TARGETS dcpp
            BUNDLE DESTINATION . )
endif (NOT APPLE)
