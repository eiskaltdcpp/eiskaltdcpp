if (UNIX AND NOT APPLE)
  project (${PROJECT_NAME_GLOBAL}-qt)
else (UNIX AND NOT APPLE)
  project (${PROJECT_NAME_GLOBAL})
endif (UNIX AND NOT APPLE)

cmake_minimum_required (VERSION 2.6)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (QT_USE_QTNETWORK 1)
set (QT_USE_QTXML 1)

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

set (SRCS
            src/ActionCustomizer.cpp
            src/ADLS.cpp
            src/ADLSModel.cpp
            src/Antispam.cpp
            src/AntiSpamFrame.cpp
            src/ArenaWidget.cpp
            src/ArenaWidgetContainer.cpp
            src/CustomFontModel.cpp
            src/DownloadQueue.cpp
            src/DownloadQueueModel.cpp
            src/EmoticonDialog.cpp
            src/EmoticonFactory.cpp
            src/FileHasher.cpp
            src/FavoriteHubModel.cpp
            src/FavoriteHubs.cpp
            src/FavoriteUsers.cpp
            src/FavoriteUsersModel.cpp
            src/FileBrowserModel.cpp
            src/FinishedTransfers.cpp
            src/FinishedTransfersModel.cpp
            src/FlowLayout.cpp
            src/HashProgress.cpp
            src/HubFrame.cpp
            src/HubManager.cpp
            src/IPFilter.cpp
            src/IPFilterFrame.cpp
            src/IPFilterModel.cpp
            src/LineEdit.cpp
            src/Magnet.cpp
            src/MainWindow.cpp
            src/MultiLineToolBar.cpp
            src/Notification.cpp
            src/PMWindow.cpp
            src/PublicHubs.cpp
            src/PublicHubsList.cpp
            src/PublicHubModel.cpp
            src/QuickConnect.cpp
            src/QueuedUsers.cpp
            src/SearchBlacklist.cpp
            src/SearchBlacklistDialog.cpp
            src/SearchFrame.cpp
            src/SearchModel.cpp
            src/Settings.cpp
            src/SettingsConnection.cpp
            src/SettingsDownloads.cpp
            src/SettingsGUI.cpp
            src/SettingsLog.cpp
            src/SettingsNotification.cpp
            src/SettingsPersonal.cpp
            src/SettingsSharing.cpp
            src/SettingsShortcuts.cpp
            src/SettingsUC.cpp
            src/ShareBrowser.cpp
            src/ShareBrowserSearch.cpp
            src/ShellCommandRunner.cpp
            src/ShortcutManager.cpp
            src/ShortcutEdit.cpp
            src/ShortcutGetter.cpp
            src/SideBar.cpp
            src/SpyFrame.cpp
            src/SpyModel.cpp
            src/TabButton.cpp
            src/TabFrame.cpp
            src/ToolBar.cpp
            src/TransferView.cpp
            src/TransferViewModel.cpp
            src/UCModel.cpp
            src/UserListModel.cpp
            src/WulforSettings.cpp
            src/WulforUtil.cpp
            src/ChatEdit.cpp
            src/qtsingleapp/qtsingleapplication.cpp
            src/qtsingleapp/qtlocalpeer.cpp
            src/qtsingleapp/qtsinglecoreapplication.cpp
            src/qtsingleapp/qtlockedfile.cpp
            )
if (WIN32)
  list (APPEND SRCS ../win32/applink.c)
  list (APPEND SRCS src/qtsingleapp/qtlockedfile_win.cpp)
else (WIN32)
  list (APPEND SRCS src/qtsingleapp/qtlockedfile_unix.cpp)
endif (WIN32)
set (MOC_HDRS
            src/ActionCustomizer.h
            src/ADLS.h
            src/ADLSModel.h
            src/Antispam.h
            src/AntiSpamFrame.h
            src/ArenaWidget.h
            src/CustomFontModel.h
            src/DownloadQueue.h
            src/DownloadQueueModel.h
            src/EmoticonDialog.h
            src/EmoticonFactory.h
            src/EmoticonObject.h
            src/FileHasher.h
            src/FavoriteHubModel.h
            src/FavoriteHubs.h
            src/FavoriteUsers.h
            src/FavoriteUsersModel.h
            src/FileBrowserModel.h
            src/FinishedTransfers.h
            src/FinishedTransfersModel.h
            src/FlowLayout.h
            src/HashProgress.h
            src/HubFrame.h
            src/HubManager.h
            src/IPFilter.h
            src/IPFilterFrame.h
            src/IPFilterModel.h
            src/LineEdit.h
            src/Magnet.h
            src/MainWindow.h
            src/MultiLineToolBar.h
            src/Notification.h
            src/PMWindow.h
            src/PublicHubs.h
            src/PublicHubsList.h
            src/PublicHubModel.h
            src/Settings.h
            src/SettingsConnection.h
            src/SettingsDownloads.h
            src/SettingsGUI.h
            src/SettingsLog.h
            src/SettingsNotification.h
            src/SettingsPersonal.h
            src/SettingsSharing.h
            src/SettingsShortcuts.h
            src/SettingsUC.h
            src/ShareBrowserSearch.h
            src/ShortcutEdit.h
            src/SideBar.h
            src/QuickConnect.h
            src/QueuedUsers.h
            src/SearchBlacklist.h
            src/SearchBlacklistDialog.h
            src/SearchFrame.h
            src/SearchModel.h
            src/ShareBrowser.h
            src/ShellCommandRunner.h
            src/ShortcutManager.h
            src/ShortcutGetter.h
            src/SpyFrame.h
            src/SpyModel.h
            src/TabButton.h
            src/TabFrame.h
            src/ToolBar.h
            src/TransferView.h
            src/TransferViewModel.h
            src/UCModel.h
            src/UserListModel.h
            src/WulforSettings.h
            src/WulforUtil.h
            src/ChatEdit.h
            src/qtsingleapp/qtlockedfile.h
            src/qtsingleapp/qtsingleapplication.h
            src/qtsingleapp/qtsinglecoreapplication.h
            src/qtsingleapp/qtlocalpeer.h
            )
set (UI_FORMS ui/UIAbout.ui
            ui/UIAntiSpam.ui
            ui/UIADLSearch.ui
            ui/UIADLSearchEditor.ui
            ui/HubFrame.ui
            ui/PrivateMessage.ui
            ui/UIDownloadQueue.ui
            ui/UIFavoriteHubs.ui
            ui/UIFavoriteHubEditor.ui
            ui/UIFavoriteUsers.ui
            ui/UIFinishedTransfers.ui
            ui/UIHashProgressDialog.ui
            ui/UIIPFilter.ui
            ui/UIMagnet.ui
            ui/UIFileHasher.ui
            ui/UIPublicHubs.ui
            ui/UIPublicHubsList.ui
            ui/UIQuickConnect.ui
            ui/UIQueuedUsers.ui
            ui/UISearchBlacklist.ui
            ui/UISearchFrame.ui
            ui/UISettings.ui
            ui/UISettingsConnection.ui
            ui/UISettingsDownloads.ui
            ui/UISettingsGUI.ui
            ui/UISettingsLog.ui
            ui/UISettingsNotification.ui
            ui/UISettingsPersonal.ui
            ui/UISettingsSharing.ui
            ui/UISettingsShortcuts.ui
            ui/UIShareBrowser.ui
            ui/UIShareBrowserSearch.ui
            ui/UISpy.ui
            ui/UITransferView.ui
            ui/UISettingsUC.ui
            ui/UIUserCommands.ui
            ui/UIScriptManager.ui
            ui/UIDialogScriptConsole.ui
            ui/UIActionCustomizer.ui
            ui/UIHubManager.ui
            )

set (WIN_RESOURCES)
if (MINGW AND WIN32)
  if (CMAKE_CROSSCOMPILING)
    set(CMAKE_RC_COMPILER i486-mingw32-windres)
  else (CMAKE_CROSSCOMPILING)
    set(CMAKE_RC_COMPILER windres)
  endif (CMAKE_CROSSCOMPILING)
  ADD_CUSTOM_COMMAND (OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/eiskaltdcpp-qt_rc.o"
            COMMAND ${CMAKE_RC_COMPILER}
            -I"${CMAKE_CURRENT_SOURCE_DIR}/"
            -i "${CMAKE_CURRENT_SOURCE_DIR}/eiskaltdcpp-qt.rc"
            -o "${CMAKE_CURRENT_BINARY_DIR}/eiskaltdcpp-qt_rc.o"
            )
  SET (WIN_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/eiskaltdcpp-qt_rc.o)
elseif (NOT MINGW AND WIN32)
  SET (WIN_RESOURCES"${CMAKE_CURRENT_SOURCE_DIR}/eiskaltdcpp-qt.rc")
endif (MINGW AND WIN32)

if (USE_ASPELL)
  list (APPEND SRCS src/SpellCheck.cpp)
  list (APPEND MOC_HDRS src/SpellCheck.h)
endif (USE_ASPELL)

if (USE_JS)
  list (APPEND SRCS
        scriptengine/ClientManagerScript.cpp
        scriptengine/HashManagerScript.cpp
        scriptengine/LogManagerScript.cpp
        scriptengine/MainWindowScript.cpp
        scriptengine/ScriptConsole.cpp
        scriptengine/ScriptEngine.cpp
        scriptengine/ScriptConsole.cpp
        src/ScriptManagerDialog.cpp)
  list (APPEND MOC_HDRS
        scriptengine/ClientManagerScript.h
        scriptengine/HashManagerScript.h
        scriptengine/LogManagerScript.h
        scriptengine/MainWindowScript.h
        scriptengine/ScriptConsole.h
        scriptengine/ScriptEngine.h
        scriptengine/ScriptConsole.h
        src/ScriptManagerDialog.h)
endif (USE_JS)

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
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LANGUAGE}.ts)
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
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/eiskaltdcpp-qt.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-qt.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
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
  set (MACOSICON "${PROJECT_SOURCE_DIR}/../icons/icon_128x128.icns")
  set (MACOSICONINBUNDLE "icon_128x128.icns")
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE ${MACOSICONINBUNDLE})
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${MACOSICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  list (APPEND SRCS ${MACOSICON})
  add_definitions(-DAPPLE)
endif (APPLE)

add_executable (eiskaltdcpp-qt MACOSX_BUNDLE WIN32 src/main.cpp
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
  target_link_libraries (eiskaltdcpp-qt ${LIBS} dcpp iconv)
else (APPLE)
  target_link_libraries (eiskaltdcpp-qt ${LIBS} dcpp)
endif (APPLE)

if (APPLE)
  set_property (TARGET eiskaltdcpp-qt PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}")
else (APPLE)
  set_property (TARGET eiskaltdcpp-qt PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-qt")
endif (APPLE)

if (UNIX)
 set (BINDIR bin)
 if (APPLE)
    set (BUNDLEDIR .)
 endif ()
elseif (WIN32)
  set (BINDIR .)
endif (UNIX)

  install (TARGETS eiskaltdcpp-qt
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
  #set (MACOSX_BUNDLE_ICON_FILE ${MACOSICONINBUNDLE})

# doesn't work this way :(
#  set (MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
  set_target_properties(eiskaltdcpp-qt PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)

  set (MACOSX_BUNDLE_GUI_IDENTIFIER "http://code.google.com/p/eiskaltdc/")
  set (MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_NAME_GLOBAL} ${VERSION})
  set (MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
  set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
  set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
  set (MACOSX_BUNDLE_COPYRIGH "Copyright 2010 Andrey Karlov")
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
  install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-qt.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
  install (FILES ${PROJECT_BINARY_DIR}/default.rcc DESTINATION ${PROJECT_NAME_GLOBAL}.app/resources)
else (APPLE)
  install (FILES ${PROJECT_BINARY_DIR}/default.rcc DESTINATION ${CLIENT_DATA_DIR}/qt/resources)
  if (UNIX)
    install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-qt.1.gz DESTINATION ${SHARE_DIR}/man/man1)
    install (FILES eiskaltdcpp-qt.desktop DESTINATION ${SHARE_DIR}/applications)
  endif (UNIX)
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
