project (dcpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)
aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)
if (HAVE_ADDRS_H EQUAL 1)
  add_definitions (-DHAVE_IFADDRS_H)
endif (HAVE_ADDRS_H EQUAL 1)
include_directories (${Boost_INCLUDEDIR} ${BZip2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${OpenSSL_INCLUDE_DIRS})
add_library (dcpp STATIC ${dcpp_srcs})
target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES})
