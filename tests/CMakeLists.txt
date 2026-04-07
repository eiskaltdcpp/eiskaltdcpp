project(eiskaltdcpp-tests)

# Fetch Catch2 v3 — use source archive instead of git clone for speed
# and resilience against transient git-protocol failures.
# Multiple mirror URLs let CMake fall back if one CDN returns a transient
# 5xx error (GitHub 502s are common on Actions runners).
include(FetchContent)
set(CATCH2_VERSION "3.5.2")
set(CATCH2_SHA256 "269543a49eb76f40b3f93ff231d4c24c27a7e16c90e47d2e45bcc564de470c6e")
FetchContent_Declare(
    Catch2
    URL        https://github.com/catchorg/Catch2/archive/refs/tags/v${CATCH2_VERSION}.tar.gz
               https://codeload.github.com/catchorg/Catch2/tar.gz/refs/tags/v${CATCH2_VERSION}
    URL_HASH   SHA256=${CATCH2_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Catch2)

# Include Catch2's CMake helpers (provides catch_discover_tests)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Test executable
add_executable(eiskaltdcpp-tests
    test_main.cpp
    test_cid.cpp
    test_util.cpp
    test_util_extended.cpp
    test_util_formatting.cpp
    test_dccontext.cpp
    test_encoder.cpp
    test_string_tokenizer.cpp
    test_text.cpp
    test_adccommand.cpp
    test_simplexml.cpp
    test_tigerhash.cpp
    test_hashbloom.cpp
    test_wildcards.cpp
    test_compression.cpp
    test_simplexmlreader.cpp
    test_adlsearch.cpp
    test_queueitem.cpp
    test_usercommand.cpp
    test_util_extra.cpp
    test_settingsmanager.cpp
    test_user.cpp
    test_searchqueue.cpp
    test_finisheditem.cpp
    test_logmanager.cpp
    test_searchresult.cpp
    test_favoritemanager.cpp
    test_merkletree.cpp
    test_nmdc_escape.cpp
    test_file_utils.cpp
    test_sfvreader.cpp
    test_streams.cpp
    test_stringsearch.cpp
    test_scopedfunctor.cpp
    test_intrusive_ptr.cpp
    test_exception.cpp
    test_version.cpp
    test_hubentry.cpp
    test_debugmanager.cpp
    test_bloomfilter.cpp
    test_flags.cpp
    test_format.cpp
    test_filteredfile.cpp
    test_dht_context.cpp
)

target_include_directories(eiskaltdcpp-tests PRIVATE
    ${CMAKE_SOURCE_DIR}          # for "dcpp/..." includes from extra/
    ${CMAKE_SOURCE_DIR}/dcpp     # for "Singleton.h" etc.
    ${BZIP2_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${GETTEXT_INCLUDE_DIR}
)
if (LUA_SCRIPT)
    target_include_directories(eiskaltdcpp-tests PRIVATE ${LUA_INCLUDE_DIR})
endif ()
if (USE_IDN2)
    target_include_directories(eiskaltdcpp-tests PRIVATE ${IDN2_INCLUDE_DIR})
endif ()

target_link_libraries(eiskaltdcpp-tests PRIVATE
    dcpp
    Catch2::Catch2WithMain
)

# Auto-discover test cases
catch_discover_tests(eiskaltdcpp-tests)

# Qt-based tests (separate binary)
if (USE_QT6)
    add_subdirectory(qt)
endif()

# GTK-based tests (separate binary, no GTK dependency — tests model layer only)
if (USE_GTK3)
    add_subdirectory(gtk)
endif()
