project (dht)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dht_srcs)

include_directories (${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${LUA_INCLUDE_DIR})

if (WITH_DHT)
  add_definitions ( -DWITH_DHT )
endif (WITH_DHT)

set (LINK STATIC)

add_library (dht ${LINK} ${dht_srcs})

if (WIN32)
    set_property(TARGET dht PROPERTY COMPILE_FLAGS)
else(WIN32)
    set_property(TARGET dht PROPERTY COMPILE_FLAGS -fPIC)
endif (WIN32)
