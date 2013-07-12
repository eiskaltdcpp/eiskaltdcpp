project (${PROJECT_NAME_GLOBAL}-qt)

cmake_minimum_required (VERSION 2.6)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (USE_QT_QML)
  find_package (Qt4 4.7.0 COMPONENTS QtCore QtGui QtXml QtNetwork QtDeclarative REQUIRED)
  message (STATUS "Building with Qt Declarative UI support")
  set (USE_JS ON)
else (USE_QT_QML)
  find_package (Qt4 4.6.0 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED)
  message (STATUS "Building without Qt Declarative UI support")
endif (USE_QT_QML)

if (APPLE OR WIN32 OR HAIKU)
    set (DBUS_NOTIFY OFF)
endif ()

if (DBUS_NOTIFY)
    find_package (Qt4 4.6.0 COMPONENTS QtDBus REQUIRED)
    message (STATUS "Building with Qt DBus support")
else (DBUS_NOTIFY)
    message (STATUS "Building without Qt DBus support")
endif (DBUS_NOTIFY)

if (USE_JS)
    find_package (Qt4 4.6.0 COMPONENTS QtScript REQUIRED)
message (STATUS "Building with Qt Script support")
else (USE_JS)
    message (STATUS "Building without Qt Script support")
endif (USE_JS)

if (USE_QT_SQLITE)
    find_package (Qt4 4.6.0 COMPONENTS QtSql REQUIRED)
    message (STATUS "Building with Qt SQLite support")
else (USE_QT_SQLITE)
    message (STATUS "Building without Qt SQLite support")
endif (USE_QT_SQLITE)

set (QT_USE_QTNETWORK 1)
set (QT_USE_QTXML 1)
set (QT_USE_QTWEBKIT 0)

if (DBUS_NOTIFY)
  set (QT_USE_QTDBUS 1)
  add_definitions (-DDBUS_NOTIFY)
endif (DBUS_NOTIFY)

if (USE_JS)
  add_definitions (-DUSE_JS)
  set (QT_USE_QTSCRIPT  1)
endif (USE_JS)

if (USE_QT_QML)
  add_definitions(-DUSE_QML)
  set (QT_USE_QTDECLARATIVE 1)
endif(USE_QT_QML)

if (WITH_DHT)
  add_definitions ( -DWITH_DHT )
endif (WITH_DHT)

if (USE_QT_SQLITE)
  set (QT_USE_QTSQL 1)

  add_definitions (-DUSE_QT_SQLITE)
endif (USE_QT_SQLITE)

include (${QT_USE_FILE})

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)

if (NOT HAIKU)
        list(APPEND SRCS src/qtsingleapp/qtsinglecoreapplication.cpp)
endif (NOT HAIKU)

file (GLOB MOC_HDRS ${PROJECT_SOURCE_DIR}/src/*.h)

if (NOT HAIKU AND NOT APPLE)
        list(APPEND MOC_HDRS
                    src/qtsingleapp/qtsinglecoreapplication.h)
        list(REMOVE_ITEM MOC_HDRS
                    ${PROJECT_SOURCE_DIR}/src/EiskaltApp_haiku.h)
        list(REMOVE_ITEM MOC_HDRS
                    ${PROJECT_SOURCE_DIR}/src/EiskaltApp_mac.h)
elseif (HAIKU)
        list(REMOVE_ITEM MOC_HDRS
                    ${PROJECT_SOURCE_DIR}/src/EiskaltApp.h)
        list(REMOVE_ITEM MOC_HDRS
                    ${PROJECT_SOURCE_DIR}/src/EiskaltApp_mac.h)
elseif (APPLE)
        list(REMOVE_ITEM MOC_HDRS
                    ${PROJECT_SOURCE_DIR}/src/EiskaltApp.h)
        list(REMOVE_ITEM MOC_HDRS
                    ${PROJECT_SOURCE_DIR}/src/EiskaltApp_mac.h)
endif ()

file (GLOB UI_FORMS ${PROJECT_SOURCE_DIR}/ui/*.ui)

if (WIN32)
  list (APPEND SRCS ../win32/applink.c)
endif (WIN32)

set (WIN_RESOURCES)
if (MINGW AND WIN32)
  ADD_CUSTOM_COMMAND (OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_rc.o"
            COMMAND ${CMAKE_RC_COMPILER}
            -I"${CMAKE_CURRENT_SOURCE_DIR}/"
            -i "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc"
            -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_rc.o"
            )
  SET (WIN_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_rc.o)
elseif (NOT MINGW AND WIN32)
  SET (WIN_RESOURCES"${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc")
endif (MINGW AND WIN32)

if (NOT USE_ASPELL)
    message (STATUS "Building without Aspell support")
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

foreach (LANGUAGE ${LANGUAGES})
  list (APPEND COMPILED_TRANSLATIONS ${PROJECT_BINARY_DIR}/translations/${LANGUAGE}.qm)
endforeach()

#Create moc files
QT4_WRAP_CPP (M_SRCS ${MOC_HDRS})
#Create .h files from ui files
QT4_WRAP_UI (U_SRCS ${UI_FORMS})
#Create translations
set (target_basename "translations_qt")
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/translations/)
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${PROJECT_SOURCE_DIR}/translations/${LANGUAGE}.ts)
        set (QM ${PROJECT_BINARY_DIR}/translations/${LANGUAGE}.qm)
        list (APPEND TRANSLATIONS ${TS})
        list (APPEND TRANSLATIONS_BINARY ${QM})
        add_custom_command (
                OUTPUT "${QM}"
                DEPENDS "${TS}"
                COMMAND ${QT_LRELEASE_EXECUTABLE}
                ARGS "${TS}" -qm "${QM}")
        add_custom_target(${target_basename}_${LANGUAGE} DEPENDS ${QM})
endforeach()

add_custom_target(${target_basename} ALL DEPENDS ${TRANSLATIONS_BINARY})

set (DEFAULT_QRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/icons/appl/default/default.qrc)

if (UNIX)
  EXECUTE_PROCESS (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-qt.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz DESTINATION ${MAN_DIR})
endif (UNIX)

EXECUTE_PROCESS (
    COMMAND ${QT_RCC_EXECUTABLE} -binary  ${DEFAULT_QRC_FILE} -o ${PROJECT_BINARY_DIR}/default.rcc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  )

include_directories (
        ${QT_INCLUDES}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/scriptengine
        ${PROJECT_SOURCE_DIR}/..
        ${Boost_INCLUDE_DIRS}
        )
list (APPEND LIBS ${QT_LIBRARIES} ${Boost_LIBRARIES} ${ICONV_LIBRARIES})

if (APPLE)
  find_library (COCOA_LIBRARY Cocoa)
  list (APPEND LIBS ${COCOA_LIBRARY})
endif (APPLE)

if (FREE_SPACE_BAR_C)
  add_definitions (-DFREE_SPACE_BAR_C)
endif (FREE_SPACE_BAR_C)

if (USE_ASPELL)
  add_definitions (-DUSE_ASPELL ${ASPELL_DEFINITIONS})
  include_directories (${ASPELL_INCLUDE_DIR})
  list (APPEND LIBS ${ASPELL_LIBRARIES})
endif (USE_ASPELL)

if (LUA_SCRIPT)
  include_directories (${LUA_INCLUDE_DIR})
  list (APPEND LIBS ${LUA_LIBRARIES})
endif (LUA_SCRIPT)

if (USE_MINIUPNP)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/SettingsConnection.cpp  PROPERTY COMPILE_DEFINITIONS USE_MINIUPNP )
endif()

if (FORCE_XDG)
   set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp PROPERTY COMPILE_DEFINITIONS FORCE_XDG )
endif (FORCE_XDG)

if (HAVE_IFADDRS_H)
  set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/WulforUtil.cpp PROPERTY COMPILE_DEFINITIONS HAVE_IFADDRS_H APPEND)
endif (HAVE_IFADDRS_H)

if (ENABLE_STACKTRACE)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp PROPERTY COMPILE_DEFINITIONS ENABLE_STACKTRACE)
endif (ENABLE_STACKTRACE)

if (APPLE)
  set (MACOSICON "${PROJECT_SOURCE_DIR}/../data/icons/icon_128x128.icns")
  set (MACOSICONINBUNDLE "icon_128x128.icns")
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE ${MACOSICONINBUNDLE})
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${MACOSICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
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
                translations
                )
add_definitions (-DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}"
                 -DCLIENT_SOUNDS_DIR="${CLIENT_SOUNDS_DEST}"
                 -DCLIENT_TRANSLATIONS_DIR="${CLIENT_TRANSLATIONS_DEST}"
                 -DCLIENT_ICONS_DIR="${CLIENT_DATA_DIR}/qt/icons"
                )

if (USE_JS)
  add_definitions (-DCLIENT_SCRIPTS_DIR="${CLIENT_SCRIPTS_DEST}")
endif (USE_JS)

if (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${PROJECT_NAME_GLOBAL}.app/resources")
else (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/qt/resources")
endif (APPLE)

if (APPLE)
  target_link_libraries (${PROJECT_NAME} ${LIBS} dcpp iconv)
elseif (HAIKU)
  target_link_libraries (${PROJECT_NAME} ${LIBS} dcpp be)
else (APPLE)
  target_link_libraries (${PROJECT_NAME} ${LIBS} dcpp)
endif (APPLE)

if (APPLE)
  set_property (TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}")
else (APPLE)
  set_property (TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}")
endif (APPLE)

  install (TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})
if (APPLE)

  set(plugin_dest_dir ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
  set(qtconf_dest_dir ${PROJECT_NAME_GLOBAL}.app/Contents/Resources)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME_GLOBAL}.app")
  set(DIRS "\${QT_LIBRARY_DIRS}" "\${PC_UPNP_LIBRARY_DIRS}" "\${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
  set(CPACK_PACKAGE_ICON ${MACOSICON})
  set(CPACK_BUNDLE_ICON ${MACOSICON})
  set(CPACK_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
  if (eiskaltdcpp_WC_REVISION MATCHES [0-9]+)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${VERSION}-${eiskaltdcpp_WC_REVISION}-${CMAKE_SYSTEM_PROCESSOR}")
  else ()
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${VERSION}-stable-${CMAKE_SYSTEM_PROCESSOR}")
  endif ()
  set (MACOSX_BUNDLE_INFO_STRING "DC and ADC client")
  set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
  set (MACOSX_BUNDLE_GUI_IDENTIFIER "http://code.google.com/p/eiskaltdc/")
  set (MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_NAME_GLOBAL} ${VERSION})
  set (MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
  set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
  set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
  set (MACOSX_BUNDLE_COPYRIGH "Copyright 2012 Andrey Karlov")
endif (APPLE)

  if (USE_JS)
    install (DIRECTORY qtscripts/ DESTINATION ${CLIENT_DATA_DIR}/qt/qtscripts
      PATTERN "*.sh"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      PATTERN "*.php"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      PATTERN ".svn" EXCLUDE )
  endif (USE_JS)

  install (DIRECTORY icons/appl icons/user DESTINATION "${CLIENT_DATA_DIR}/qt/icons" PATTERN ".svn" EXCLUDE)
  install (FILES ${TRANSLATIONS_BINARY} DESTINATION ${CLIENT_TRANSLATIONS_DEST})
if (APPLE)
  install (FILES eiskaltdcpp-qt.desktop DESTINATION ${PROJECT_NAME_GLOBAL}.app/applications)
  install (FILES ${PROJECT_BINARY_DIR}/default.rcc DESTINATION ${PROJECT_NAME_GLOBAL}.app/resources)
else (APPLE)
  install (FILES ${PROJECT_BINARY_DIR}/default.rcc DESTINATION ${CLIENT_DATA_DIR}/qt/resources)
  if (UNIX AND NOT HAIKU)
    install (FILES ${PROJECT_NAME}.desktop DESTINATION ${SHARE_DIR}/applications)
  endif (UNIX AND NOT HAIKU)
endif (APPLE)

if (APPLE)
  install (DIRECTORY ${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib DESTINATION ${PROJECT_NAME_GLOBAL}.app/Contents/Resources)

  INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")")

  INSTALL(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")")
    set(CPACK_BINARY_DRAGNDROP ON)

endif (APPLE)

include(CPack)
