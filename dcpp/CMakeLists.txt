project (dcpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)

if (HAVE_ADDRS_H EQUAL 1)
  add_definitions (-DHAVE_IFADDRS_H)
endif (HAVE_ADDRS_H EQUAL 1)

include_directories (${Boost_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

add_library (dcpp SHARED ${dcpp_srcs})
target_link_libraries (dcpp ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES})

install (TARGETS dcpp
	 LIBRARY DESTINATION lib
	 BUNDLE DESTINATION . )