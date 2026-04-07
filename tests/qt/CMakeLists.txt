project(eiskaltdcpp-qt-tests)

# --- Catch2 (already fetched in parent if BUILD_TESTS is on) ---
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# --- Qt6 ---
find_package(Qt6 6.2.0 COMPONENTS Core Gui Widgets Test REQUIRED)

# Enable automoc for Q_OBJECT classes
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# --- Qt source files under test (compiled into the test binary) ---
set(QT_SRC_DIR ${CMAKE_SOURCE_DIR}/eiskaltdcpp-qt/src)

set(QT_MODEL_SOURCES
    ${QT_SRC_DIR}/QtContext.cpp
    ${QT_SRC_DIR}/FavoriteHubModel.cpp
    ${QT_SRC_DIR}/ADLSModel.cpp
    ${QT_SRC_DIR}/IPFilterModel.cpp
    ${QT_SRC_DIR}/SpyModel.cpp
    ${QT_SRC_DIR}/FlowLayout.cpp
    ${QT_SRC_DIR}/WulforSettings.cpp
    ${QT_SRC_DIR}/SearchBlacklist.cpp
    ${QT_SRC_DIR}/Antispam.cpp
)

# --- Test sources ---
add_executable(eiskaltdcpp-qt-tests
    test_qt_main.cpp
    test_favoritehubmodel.cpp
    test_adlsmodel.cpp
    test_ipfiltermodel.cpp
    test_spymodel.cpp
    test_flowlayout.cpp
    test_wulforsettings.cpp
    test_searchblacklist.cpp
    test_antispam.cpp
    stubs_wulforutil.cpp
    ${QT_MODEL_SOURCES}
)

target_include_directories(eiskaltdcpp-qt-tests PRIVATE
    ${QT_SRC_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dcpp
    ${GETTEXT_INCLUDE_DIR}
)
target_compile_definitions(eiskaltdcpp-qt-tests PRIVATE QT_CONTEXT_MINIMAL)
if (LUA_SCRIPT)
    target_include_directories(eiskaltdcpp-qt-tests PRIVATE ${LUA_INCLUDE_DIR})
endif()

target_link_libraries(eiskaltdcpp-qt-tests PRIVATE
    dcpp
    Catch2::Catch2
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

# Auto-discover test cases.
# - PRE_TEST defers discovery to ctest time (avoids build-time env issues)
# - ENVIRONMENT sets offscreen rendering for headless CI runners
catch_discover_tests(eiskaltdcpp-qt-tests
    DISCOVERY_MODE PRE_TEST
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
