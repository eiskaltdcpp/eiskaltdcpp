project(eiskaltdcpp-gtk-tests)

# --- Catch2 (already fetched in parent if BUILD_TESTS is on) ---
include(FetchContent)
FetchContent_Declare(
    Catch2
    URL        https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.tar.gz
    URL_HASH   SHA256=269543a49eb76f40b3f93ff231d4c24c27a7e16c90e47d2e45bcc564de470c6e
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# --- GTK model sources under test ---
set(GTK_MODELS_DIR ${CMAKE_SOURCE_DIR}/eiskaltdcpp-gtk/src/models)

# --- Test sources ---
add_executable(eiskaltdcpp-gtk-tests
    test_gtk_main.cpp
    test_gtkformatters.cpp
    test_gtkwulforutil.cpp
    test_messagetypes.cpp
    test_emoticonloader.cpp
    test_soundnotify.cpp
    test_hubuserparams.cpp
    test_searchresultparams.cpp
    test_downloadqueueparams.cpp
    test_favoritehubparams.cpp
    test_finishedtransferparams.cpp
    test_transferparams.cpp
    test_favoritehublistmodel.cpp
    test_hubuserlistmodel.cpp
    test_transferlistmodel.cpp
    test_downloadqueuemodel.cpp
    test_searchresultsmodel.cpp
    test_gtksettingsmodel.cpp
    test_chatformatter.cpp
    test_publichubfilter.cpp
    test_adlsearchdisplay.cpp
    test_sharebrowsermodel.cpp
    test_settingsvalidation.cpp
    test_gtksettingsmanager.cpp
)

target_include_directories(eiskaltdcpp-gtk-tests PRIVATE
    ${GTK_MODELS_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dcpp
    ${CMAKE_SOURCE_DIR}/extra
    ${CMAKE_SOURCE_DIR}/tests       # for TestContext.h
    ${BZIP2_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${GETTEXT_INCLUDE_DIR}
)
if (LUA_SCRIPT)
    target_include_directories(eiskaltdcpp-gtk-tests PRIVATE ${LUA_INCLUDE_DIR})
endif()
if (USE_IDN2)
    target_include_directories(eiskaltdcpp-gtk-tests PRIVATE ${IDN2_INCLUDE_DIR})
endif()

target_link_libraries(eiskaltdcpp-gtk-tests PRIVATE
    gtkmodels
    dcpp
    Catch2::Catch2WithMain
)

# Auto-discover test cases
catch_discover_tests(eiskaltdcpp-gtk-tests)
