project (dht)

cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dht_srcs)

set (DHT_SRCS ${dht_srcs} PARENT_SCOPE)

include_directories (${PROJECT_BINARY_DIR}
                     ${OPENSSL_INCLUDE_DIR}
                     ${GETTEXT_INCLUDE_DIR}
                     ${BZIP2_INCLUDE_DIR}
                     ${ZLIB_INCLUDE_DIR}
                     ${LUA_INCLUDE_DIR})

set (LINK STATIC)

add_library (dht ${LINK} ${dht_srcs})

if (WIN32)
    set_property(TARGET dht PROPERTY COMPILE_FLAGS)
else(WIN32)
    set_property(TARGET dht PROPERTY COMPILE_FLAGS -fPIC)
endif (WIN32)

