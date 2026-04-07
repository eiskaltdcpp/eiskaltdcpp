cmake_minimum_required (VERSION 3.10)
project (eiskaltdcpp-qt)

include_directories(${PROJECT_BINARY_DIR})

# --- Qt version selection ---
set (QT_MAJOR 6)
set (QT_MIN_VERSION 6.2.0)
set (QT "Qt${QT_MAJOR}")
message (STATUS "Building eiskaltdcpp-qt with ${QT}")

# --- Find required Qt modules ---
find_package (${QT} ${QT_MIN_VERSION} COMPONENTS Core Gui Widgets Xml Network
              Concurrent REQUIRED)
# Multimedia is optional (needed for QSoundEffect notification sounds)
find_package (${QT} COMPONENTS Multimedia QUIET)
if (${QT}Multimedia_FOUND)
    message (STATUS "Found ${QT}::Multimedia — sound notifications enabled")
else ()
    message (STATUS "${QT}::Multimedia not found — sound notifications disabled")
    add_definitions (-DNO_QT_MULTIMEDIA)
endif ()
# LinguistTools is optional (only needed for updating translations)
find_package (${QT} COMPONENTS LinguistTools QUIET)

# --- QDeclarativeView removed in Qt6, replaced with QQuickWidget ---
if (USE_QT_QML)
    find_package (${QT} ${QT_MIN_VERSION} COMPONENTS Quick QuickWidgets REQUIRED)
    add_definitions (-DUSE_QML)
    set (USE_JS ON)
    message (STATUS "Building with Qt Quick UI support")
else (USE_QT_QML)
    message (STATUS "Building without Qt Quick/Declarative UI support")
endif (USE_QT_QML)

if (APPLE OR HAIKU)
    set (DBUS_NOTIFY OFF)
endif ()

if (DBUS_NOTIFY)
    find_package (${QT} ${QT_MIN_VERSION} COMPONENTS DBus REQUIRED)
    add_definitions (-DDBUS_NOTIFY)
    message (STATUS "Building with Qt DBus support")
else (DBUS_NOTIFY)
    message (STATUS "Building without Qt DBus support")
endif (DBUS_NOTIFY)

if (USE_JS)
    find_package (${QT} ${QT_MIN_VERSION} COMPONENTS Qml REQUIRED)
    add_definitions (-DUSE_JS)
    message (STATUS "Building with Qt Script support")
else (USE_JS)
    message (STATUS "Building without Qt Script support")
endif (USE_JS)

if (USE_QT_SQLITE)
    find_package (${QT} ${QT_MIN_VERSION} COMPONENTS Sql REQUIRED)
    add_definitions (-DUSE_QT_SQLITE)
    message (STATUS "Building with Qt SQLite support")
else (USE_QT_SQLITE)
    message (STATUS "Building without Qt SQLite support")
endif (USE_QT_SQLITE)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)

if (NOT HAIKU)
    list (APPEND SRCS src/qtsingleapp/qtsinglecoreapplication.cpp)
endif (NOT HAIKU)

file (GLOB MOC_HDRS ${PROJECT_SOURCE_DIR}/src/*.h)

if (NOT HAIKU AND NOT APPLE)
    list (APPEND MOC_HDRS
                src/qtsingleapp/qtsinglecoreapplication.h)
    list (REMOVE_ITEM MOC_HDRS
                ${PROJECT_SOURCE_DIR}/src/EiskaltApp_haiku.h)
    list (REMOVE_ITEM MOC_HDRS
                ${PROJECT_SOURCE_DIR}/src/EiskaltApp_mac.h)
elseif (HAIKU)
    list (REMOVE_ITEM MOC_HDRS
                ${PROJECT_SOURCE_DIR}/src/EiskaltApp.h)
    list (REMOVE_ITEM MOC_HDRS
                ${PROJECT_SOURCE_DIR}/src/EiskaltApp_mac.h)
elseif (APPLE)
    list (APPEND MOC_HDRS
                src/qtsingleapp/qtsinglecoreapplication.h)
    list (REMOVE_ITEM MOC_HDRS
                ${PROJECT_SOURCE_DIR}/src/EiskaltApp.h)
    list (REMOVE_ITEM MOC_HDRS
                ${PROJECT_SOURCE_DIR}/src/EiskaltApp_haiku.h)
endif (NOT HAIKU AND NOT APPLE)

file (GLOB UI_FORMS ${PROJECT_SOURCE_DIR}/ui/*.ui)

set (WIN_RESOURCES)
if (MINGW AND WIN32)
    add_custom_command (OUTPUT "${PROJECT_BINARY_DIR}/eiskaltdcpp-qt_rc.o"
                        COMMAND ${CMAKE_RC_COMPILER}
                                -I"${PROJECT_SOURCE_DIR}/"
                                -i "${PROJECT_SOURCE_DIR}/eiskaltdcpp-qt.rc"
                                -o "${PROJECT_BINARY_DIR}/eiskaltdcpp-qt_rc.o")
    set (WIN_RESOURCES ${PROJECT_BINARY_DIR}/eiskaltdcpp-qt_rc.o)
elseif (NOT MINGW AND WIN32)
    set (WIN_RESOURCES "${PROJECT_SOURCE_DIR}/eiskaltdcpp-qt.rc")
endif (MINGW AND WIN32)

if (NOT USE_ASPELL)
    message (STATUS "Building without Aspell support")
    set (LOCAL_ASPELL_DATA OFF)
elseif (USE_ASPELL)
    find_package (ASPELL REQUIRED)
    message (STATUS "Building with Aspell support")
endif (NOT USE_ASPELL)

if (NOT ASPELL_FOUND)
    list (REMOVE_ITEM SRCS ${PROJECT_SOURCE_DIR}/src/SpellCheck.cpp)
    list (REMOVE_ITEM MOC_HDRS ${PROJECT_SOURCE_DIR}/src/SpellCheck.h)
endif (NOT ASPELL_FOUND)

if (USE_JS)
    file (GLOB scriptengine_srcs ${PROJECT_SOURCE_DIR}/scriptengine/*.cpp)
    file (GLOB scriptengine_hdrs ${PROJECT_SOURCE_DIR}/scriptengine/*.h)

    list (APPEND SRCS
            ${scriptengine_srcs}
            src/codeeditor/codeeditor.cpp)
    list (APPEND MOC_HDRS
            ${scriptengine_hdrs}
            src/codeeditor/codeeditor.h)
endif (USE_JS)

if (NOT USE_JS)
    list (REMOVE_ITEM UI_FORMS
            ${PROJECT_SOURCE_DIR}/ui/UIScriptManager.ui
            ${PROJECT_SOURCE_DIR}/ui/UIDialogScriptConsole.ui)
    list (REMOVE_ITEM SRCS
            ${PROJECT_SOURCE_DIR}/src/ScriptManagerDialog.cpp)
    list (REMOVE_ITEM MOC_HDRS
            ${PROJECT_SOURCE_DIR}/src/ScriptManagerDialog.h)
endif (NOT USE_JS)

qt_wrap_cpp (M_SRCS ${MOC_HDRS} OPTIONS -nn)
qt_wrap_ui (U_SRCS ${UI_FORMS})

if (TARGET ${QT}::lrelease)
    get_target_property (QT_LRELEASE_EXECUTABLE ${QT}::lrelease LOCATION)
else ()
    find_program (QT_LRELEASE_EXECUTABLE NAMES lrelease-qt6 lrelease6 lrelease)
endif ()
get_target_property (QT_RCC_EXECUTABLE ${QT}::rcc LOCATION)

#Create translations
file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/translations/)
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${PROJECT_SOURCE_DIR}/translations/${LANGUAGE}.ts)
        set (QM ${PROJECT_BINARY_DIR}/translations/${LANGUAGE}.qm)
        list (APPEND TRANSLATIONS ${TS})
        list (APPEND TRANSLATIONS_BINARY ${QM})
        add_custom_command (OUTPUT "${QM}"
                            DEPENDS "${TS}"
                            COMMAND ${QT_LRELEASE_EXECUTABLE}
                            ARGS "${TS}" -qm "${QM}")
endforeach ()

if (WIN32 OR APPLE OR INSTALL_QT_TRANSLATIONS OR BUILD_STATIC)
    # Qt translations
    get_target_property (QtCore_location ${QT}::Core LOCATION)
    get_filename_component (QtCore_DIR "${QtCore_location}" PATH)
    if (WIN32)
        set (QT_ROOT_DIR "${QtCore_DIR}/..")
    elseif (APPLE)
        set (QT_ROOT_DIR "${QtCore_DIR}/../..")
    elseif (BUILD_STATIC)
        set (QT_ROOT_DIR "${QtCore_DIR}/..")
    elseif (INSTALL_QT_TRANSLATIONS AND NOT BUILD_STATIC)
        set (QT_ROOT_DIR "/usr/share/qt6")
    endif ()
    set (QT_TRANSLATIONS_DIR "${QT_ROOT_DIR}/translations/")
    file (GLOB QT_TRANSLATIONS "${QT_TRANSLATIONS_DIR}/qt_*.qm")
    foreach (FILE ${QT_TRANSLATIONS})
        if (NOT FILE MATCHES "_help_")
            list (APPEND TRANSLATIONS_BINARY ${FILE})
        endif ()
    endforeach ()
    file (GLOB QT_TRANSLATIONS "${QT_TRANSLATIONS_DIR}/qtbase_*.qm")
    list (APPEND TRANSLATIONS_BINARY ${QT_TRANSLATIONS})
endif ()

add_custom_target(${PROJECT_NAME}_tr ALL DEPENDS ${TRANSLATIONS_BINARY})

set (DEFAULT_QRC_FILE ${PROJECT_SOURCE_DIR}/icons/appl/default/default.qrc)

if (UNIX AND NOT APPLE AND NOT HAIKU)
    if(COMPRESS_MANPAGES)
        execute_process (OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                         COMMAND "${GZIP_CMD}" -9
                         INPUT_FILE ${PROJECT_NAME}.1
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/)
        install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz DESTINATION ${MAN_DIR})
    else(COMPRESS_MANPAGES)
        install (FILES ${PROJECT_NAME}.1 DESTINATION ${MAN_DIR})
    endif(COMPRESS_MANPAGES)
endif ()

if (WIN32 OR APPLE)
    install (FILES manual.${PROJECT_NAME}.html DESTINATION "${CLIENT_DOCS_DIR}" RENAME manual.${PROJECT_NAME_GLOBAL}.html)
endif ()

execute_process (COMMAND ${QT_RCC_EXECUTABLE} -binary  ${DEFAULT_QRC_FILE}
                                              -o ${PROJECT_BINARY_DIR}/default.rcc
                 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/)

# Symlink icons from source tree into build tree so the binary can find
# user icons (usericons.png) and app icons when running uninstalled.
if (NOT EXISTS "${PROJECT_BINARY_DIR}/icons")
    execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink
                     "${PROJECT_SOURCE_DIR}/icons"
                     "${PROJECT_BINARY_DIR}/icons")
endif ()

# Qt target-based linking (handles include_directories automatically)
list (APPEND LIBS ${QT}::Core
                  ${QT}::Gui
                  ${QT}::Widgets
                  ${QT}::Xml
                  ${QT}::Concurrent
                  ${QT}::Network)
if (${QT}Multimedia_FOUND)
    list (APPEND LIBS ${QT}::Multimedia)
endif ()
if (USE_QT_QML)
    list (APPEND LIBS ${QT}::Quick ${QT}::QuickWidgets)
endif ()
if (DBUS_NOTIFY)
    list (APPEND LIBS ${QT}::DBus)
endif ()
if (USE_JS)
    list (APPEND LIBS ${QT}::Qml)
endif ()
if (USE_QT_SQLITE)
    list (APPEND LIBS ${QT}::Sql)
endif ()

include_directories (${PROJECT_SOURCE_DIR}
                     ${PROJECT_SOURCE_DIR}/src
                     ${PROJECT_SOURCE_DIR}/scriptengine
                     ${PROJECT_SOURCE_DIR}/..
                     ${GETTEXT_INCLUDE_DIR})
list (APPEND LIBS ${ICONV_LIBRARIES})

if (WIN32)
    if (OPENSSL_MSVC)
        list (APPEND SRCS ../windows/applink.c)
    endif (OPENSSL_MSVC)
endif (WIN32)

if (APPLE)
    find_library (COCOA_LIBRARY Cocoa)
    list (APPEND LIBS ${COCOA_LIBRARY})
endif (APPLE)

if (USE_PROGRESS_BARS)
    add_definitions (-DUSE_PROGRESS_BARS)
endif (USE_PROGRESS_BARS)

if (FREE_SPACE_BAR_C)
    add_definitions (-DFREE_SPACE_BAR_C)
endif (FREE_SPACE_BAR_C)

if (USE_ASPELL)
    include_directories (${ASPELL_INCLUDE_DIR})
    list (APPEND LIBS ${ASPELL_LIBRARIES})
    add_definitions (-DUSE_ASPELL ${ASPELL_DEFINITIONS})
    if (LOCAL_ASPELL_DATA)
        add_definitions (-DLOCAL_ASPELL_DATA)
    endif (LOCAL_ASPELL_DATA)
endif (USE_ASPELL)

if (LUA_SCRIPT)
    include_directories (${LUA_INCLUDE_DIR})
    list (APPEND LIBS ${LUA_LIBRARIES})
endif (LUA_SCRIPT)

if (USE_MINIUPNP)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/SettingsConnection.cpp
                 PROPERTY COMPILE_DEFINITIONS USE_MINIUPNP )
endif()

if (FORCE_XDG)
   set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp
                PROPERTY COMPILE_DEFINITIONS FORCE_XDG )
endif (FORCE_XDG)

if (HAVE_IFADDRS_H)
  set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/WulforUtil.cpp
               PROPERTY COMPILE_DEFINITIONS HAVE_IFADDRS_H APPEND)
endif (HAVE_IFADDRS_H)

if (ENABLE_STACKTRACE)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp
                 PROPERTY COMPILE_DEFINITIONS ENABLE_STACKTRACE)
endif (ENABLE_STACKTRACE)

if (APPLE)
    set (MACOSICON "${PROJECT_SOURCE_DIR}/../data/icons/icon_1024x1024.icns")
    set (MACOSICONINBUNDLE "icon_1024x1024.icns")
    # Set how it shows up in the Info.plist file
    set (MACOSX_BUNDLE_ICON_FILE ${MACOSICONINBUNDLE})
    # Set where in the bundle to put the icns file
    set_source_files_properties(${MACOSICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    # Include the icns file in the target
    list (APPEND SRCS ${MACOSICON})
endif (APPLE)

add_executable (${PROJECT_NAME} MACOSX_BUNDLE WIN32 src/main.cpp
                ${M_SRCS}
                ${U_SRCS}
                ${SRCS}
                ${HDRS}
                ${MOC_HDRS}
                ${TRANSLATIONS}
                ${WIN_RESOURCES}
                translations)

# Embed application manifest for proper DLL loading, DPI awareness, and
# Windows version compatibility.  For MSVC, CMake passes .manifest sources
# to mt.exe for merging with its auto-generated manifest.  Embedding via
# the RC file would cause a duplicate-resource link error (LNK1123).
if (WIN32 AND MSVC)
    target_sources (${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/windows/eiskaltdcpp.manifest)
endif ()

add_definitions (-DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}")
add_definitions (-DCLIENT_SOUNDS_DIR="${CLIENT_SOUNDS_DEST}")
add_definitions (-DCLIENT_TRANSLATIONS_DIR="${CLIENT_TRANSLATIONS_DEST}")

if (DEBUG_QT_UI)
    add_definitions (-D_DEBUG_QT_UI)
    message (STATUS "Enabled debug output from eiskaltdcpp-qt!")
endif (DEBUG_QT_UI)
if (DEBUG_IPFILTER)
    add_definitions (-D_DEBUG_IPFILTER)
endif (DEBUG_IPFILTER)

if (USE_JS)
    if (UNIX AND NOT APPLE AND NOT HAIKU)
        set (CLIENT_SCRIPTS_DEST "${CLIENT_DATA_DIR}/qt/qtscripts")
    else ()
        set (CLIENT_SCRIPTS_DEST "${CLIENT_DATA_DIR}/qtscripts")
    endif ()
    add_definitions (-DCLIENT_SCRIPTS_DIR="${CLIENT_SCRIPTS_DEST}")
endif (USE_JS)

if (WIN32)
    add_definitions (-DCLIENT_ICONS_DIR="${CLIENT_DATA_DIR}/icons")
    add_definitions (-DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/client-res")
elseif (HAIKU)
    add_definitions (-DCLIENT_ICONS_DIR="icons")
    add_definitions (-DCLIENT_RES_DIR="client-res")
elseif (APPLE)
    add_definitions (-DCLIENT_ICONS_DIR="icons")
    add_definitions (-DCLIENT_RES_DIR="client-res")
else ()
    add_definitions (-DCLIENT_ICONS_DIR="${CLIENT_DATA_DIR}/qt/icons")
    add_definitions (-DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/qt/client-res")
endif ()

if (APPLE)
    target_link_libraries (${PROJECT_NAME} ${LIBS} extra dcpp iconv)
elseif (HAIKU)
    target_link_libraries (${PROJECT_NAME} ${LIBS} extra dcpp be)
else (APPLE)
    target_link_libraries (${PROJECT_NAME} ${LIBS} extra dcpp)
endif (APPLE)

if (WIN32 OR APPLE OR HAIKU)
    set_property (TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}")
else ()
    set_property (TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}")
endif ()

install (TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

if (APPLE)
    set (plugin_dest_dir ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
    set (qtconf_dest_dir ${PROJECT_NAME_GLOBAL}.app/Contents/Resources)
    set (APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME_GLOBAL}.app")
    set (DIRS "\${QT_LIBRARY_DIRS}" "\${PC_UPNP_LIBRARY_DIRS}" "\${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
    set (CPACK_PACKAGE_ICON ${MACOSICON})
    set (CPACK_BUNDLE_ICON ${MACOSICON})
    set (CPACK_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        string (REGEX REPLACE "^(.*-.*)-g.*$" "\\1-macOS${CMAKE_OSX_DEPLOYMENT_TARGET}" FILE_NAME_VERSION "${APP_VERSION}")
        set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${FILE_NAME_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
    else (CMAKE_OSX_DEPLOYMENT_TARGET)
        set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${APP_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
    endif (CMAKE_OSX_DEPLOYMENT_TARGET)
    set_target_properties (${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/Info.plist.in)
    set (MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.eiskaltdcpp")
    set (MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_NAME_GLOBAL} ${APP_VERSION})
    set (MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
    set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION})
    set (MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION})
    set (MACOSX_BUNDLE_COPYRIGHT "Copyright 2009-2021 EiskaltDC++ developers")
endif (APPLE)

if (USE_JS)
    install (DIRECTORY qtscripts/ DESTINATION ${CLIENT_SCRIPTS_DEST}
             PATTERN "*.sh"
             PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
             PATTERN "*.php"
             PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (USE_JS)

install (FILES ${TRANSLATIONS_BINARY} DESTINATION ${CLIENT_TRANSLATIONS_DEST})

if (UNIX AND NOT APPLE AND NOT HAIKU)
    install (DIRECTORY icons/appl icons/user DESTINATION "${CLIENT_DATA_DIR}/qt/icons")
    install (FILES ${PROJECT_BINARY_DIR}/default.rcc DESTINATION "${CLIENT_DATA_DIR}/qt/client-res")
else ()
    install (DIRECTORY icons/appl icons/user DESTINATION "${CLIENT_DATA_DIR}/icons")
    install (FILES ${PROJECT_BINARY_DIR}/default.rcc DESTINATION "${CLIENT_DATA_DIR}/client-res")
endif ()

if (UNIX AND NOT APPLE AND NOT HAIKU)
    install (FILES ${PROJECT_NAME}.desktop DESTINATION ${SHARE_DIR}/applications)
endif ()

if (INSTALL_METAINFO)
    install (FILES ${PROJECT_NAME}.appdata.xml DESTINATION ${SHARE_DIR}/metainfo)
endif ()

if (APPLE)
    get_target_property (_qt_install_prefix ${QT}::Core LOCATION)
    get_filename_component (_qt_install_prefix "${_qt_install_prefix}" PATH)
    get_filename_component (_qt_install_prefix "${_qt_install_prefix}" PATH)
    set (MACDEPLOYQT "${_qt_install_prefix}/bin/macdeployqt")
    install (CODE "
                execute_process(COMMAND ${MACDEPLOYQT} ${PROJECT_NAME_GLOBAL}.app -verbose=2
                    WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX})")
    set (CPACK_BINARY_DRAGNDROP ON)
endif (APPLE)
