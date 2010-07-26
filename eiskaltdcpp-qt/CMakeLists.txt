project (${PROJECT_NAME_GLOBAL})
cmake_minimum_required (VERSION 2.6)
if (FREE_SPACE_BAR_C)
add_subdirectory(fsusage)
endif (FREE_SPACE_BAR_C)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (QT_USE_QTNETWORK 1)
set (QT_USE_QTXML 1)

if (DBUS_NOTIFY)
set (QT_USE_QTDBUS 1)
add_definitions (-DDBUS_NOTIFY)
endif (DBUS_NOTIFY)

if (USE_JS)
  set (QT_USE_QTSCRIPT  1)
endif (USE_JS)

include (${QT_USE_FILE})
#set (HDRS   src/ArenaWidgetContainer.h
#            src/EmoticonObject.h
#            src/Func.h
#            src/HistoryInterface.h
#            src/PMWindow.h
#            src/PoolAlloc.h
#            ../Version.h
#            )
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
            src/SettingsUC.cpp
            src/ShareBrowser.cpp
            src/ShellCommandRunner.cpp
            src/SideBar.cpp
            src/SingleInstanceRunner.cpp
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
            )
if (WIN32)
  list (APPEND SRCS openssl/applink.c)
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
            src/SettingsUC.h
            src/SideBar.h
            src/SingleInstanceRunner.h
            src/QuickConnect.h
            src/SearchFrame.h
            src/SearchModel.h
            src/ShareBrowser.h
            src/ShellCommandRunner.h
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
            ui/UIPublicHubs.ui
            ui/UIPublicHubsList.ui
            ui/UIQuickConnect.ui
            ui/UISearchFrame.ui
            ui/UISettings.ui
            ui/UISettingsConnection.ui
            ui/UISettingsDownloads.ui
            ui/UISettingsGUI.ui
            ui/UISettingsLog.ui
            ui/UISettingsNotification.ui
            ui/UISettingsPersonal.ui
            ui/UISettingsSharing.ui
            ui/UIShareBrowser.ui
            ui/UISpy.ui
            ui/UITransferView.ui
            ui/UISettingsUC.ui
            ui/UIUserCommands.ui
            ui/UIScriptManager.ui
            ui/UIDialogScriptConsole.ui
            ui/UIActionCustomizer.ui
            )

set (WIN_RESOURCES)
if (MINGW AND WIN32)
  ADD_CUSTOM_COMMAND (OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/eiskaltdcpp-qt_rc.o"
            COMMAND windres.exe
            -I"${CMAKE_CURRENT_SOURCE_DIR}/"
            -i "${CMAKE_CURRENT_SOURCE_DIR}/eiskaltdcpp-qt.rc"
            -o "${CMAKE_CURRENT_BINARY_DIR}/eiskaltdcpp-qt_rc.o"
            )
  SET (WIN_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/eiskaltdcpp-qt_rc.o)
elseif (NOT MINGW AND WIN32)
  SET (WIN_RESOURCES"${CMAKE_CURRENT_SOURCE_DIR}/eiskaltdcpp-qt.rc")
endif (MINGW AND WIN32)

option (LANGUAGES "Using translations")
set (AllLanguages
            be
            en
            es
            fr
            hu
            pl
            ru
            sr
            uk
            )
if (NOT DEFINED linguas)
  set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
else (NOT DEFINED linguas)
  if (NOT linguas)
    set (LANGUAGES "" CACHE STRING "Using translations" FORCE)
  elseif (linguas STREQUAL *)
        set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
  else (NOT linguas)
    string(REGEX MATCHALL [a-zA-Z_]+
           linguas1 ${linguas})
    set (LANGUAGES ${linguas1} CACHE STRING "Using translations" FORCE)
  endif (NOT linguas)
endif (NOT DEFINED linguas)
message (STATUS "Translations: ${LANGUAGES}")

if (USE_ASPELL)
  list (APPEND SRCS src/SpellCheck.cpp)
  list (APPEND MOC_HDRS src/SpellCheck.h)
endif (USE_ASPELL)
if (USE_LIBUPNP)
    list (APPEND SRCS src/UPnP.cpp src/UPnPMapper.cpp)
    list (APPEND MOC_HDRS  src/UPnP.h src/UPnPMapper.h)
endif (USE_LIBUPNP)
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
  list (APPEND COMPILED_TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LANGUAGE}.qm)
  #message (STATUS ${COMPILED_TRANSLATIONS})
endforeach()

#Create moc files
QT4_WRAP_CPP (M_SRCS ${MOC_HDRS})
#Create .h files from ui files
QT4_WRAP_UI (U_SRCS ${UI_FORMS})
#Create translations
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LANGUAGE}.ts)
        set (QM ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LANGUAGE}.qm)
        list (APPEND TRANSLATIONS ${TS})
        list (APPEND TRANSLATIONS_BINARY ${QM})
        add_custom_command (
                OUTPUT ${QM}
                COMMAND ${QT_LRELEASE_EXECUTABLE} ${TS}
                MAIN_DEPENDENCY ${TS})
endforeach()

set (DEFAULT_QRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/icons/appl/default/default.qrc)

add_custom_target (
        translations_qt
        COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} -ts ${TRANSLATIONS})
add_custom_command (
        TARGET translations_qt
        COMMAND ${QT_LRELEASE_EXECUTABLE} ${TRANSLATIONS})

if (UNIX)
  FIND_PROGRAM (GZIP_CMD gzip)
  EXECUTE_PROCESS (
                OUTPUT_FILE eiskaltdcpp-qt.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-qt.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

if (UNIX)
  EXECUTE_PROCESS (
    COMMAND ${QT_RCC_EXECUTABLE} -binary  ${DEFAULT_QRC_FILE} -o default.rcc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  )
endif (UNIX)

include_directories (
        ${QT_INCLUDES}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/scriptengine
        ${PROJECT_SOURCE_DIR}/..
        ${Boost_INCLUDE_DIRS}
        )
list (APPEND LIBS ${QT_LIBRARIES} ${Boost_LIBRARIES} ${ICONV_LIBRARIES})
if (USE_LIBUPNP)
include_directories (${UPNP_INCLUDE_DIR})
list (APPEND LIBS ${UPNP_LIBRARY})
add_definitions (-DUSE_LIBUPNP)
endif (USE_LIBUPNP)

if (FREE_SPACE_BAR_C)
  add_definitions (-DFREE_SPACE_BAR_C)
  if (NOT WIN32)
    list (APPEND LIBS fsusage)
  endif (NOT WIN32)
endif (FREE_SPACE_BAR_C)

if (USE_ASPELL)
  add_definitions (-DUSE_ASPELL ${ASPELL_DEFINITIONS})
  include_directories (${ASPELL_INCLUDE_DIR})
  list (APPEND LIBS ${ASPELL_LIBRARIES})
endif (USE_ASPELL)

if (APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE icon_128x128.icns)
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  list (APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns)
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
                 -DCLIENT_ICONS_DIR="${CLIENT_ICONS_DEST}"
                )

if (USE_JS)
  add_definitions (-DCLIENT_SCRIPTS_DIR="${CLIENT_SCRIPTS_DEST}")
endif (USE_JS)

if (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${PROJECT_NAME_GLOBAL}.app/resources")
else (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/resources")
endif (APPLE)

if (APPLE)
  target_link_libraries (eiskaltdcpp-qt ${LIBS} dcpp iconv upnp ixml)
else (APPLE)
  target_link_libraries (eiskaltdcpp-qt ${LIBS} dcpp)
endif (APPLE)

if (UNIX AND NOT APPLE)
  set_property (TARGET eiskaltdcpp-qt PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-qt")
else ()
  set_property (TARGET eiskaltdcpp-qt PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL} Qt")
endif ()

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
  set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns)
  set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns)
  set(CPACK_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
  if (eiskaltdcpp_WC_REVISION MATCHES [0-9]+)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${VERSION}-${eiskaltdcpp_WC_REVISION}")
  else ()
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${VERSION}-stable")
  endif ()
  set (MACOSX_BUNDLE_INFO_STRING "DC and ADC client")
  set (MACOSX_BUNDLE_ICON_FILE icon_128x128.icns)

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
    install (DIRECTORY scripts/ DESTINATION ${CLIENT_DATA_DIR}/scripts
      PATTERN "*.sh"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      PATTERN "*.php"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      PATTERN ".svn" EXCLUDE )
  endif (USE_JS)
if (WITH_EXAMPLES)
  install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    PATTERN ".svn" EXCLUDE )
endif (WITH_EXAMPLES)



if (UNIX AND NOT APPLE)
  set (CLIENT_ICONS_DEST1 ${SHARE_DIR}/icons)
  set (CLIENT_ICONS_DEST2 ${SHARE_DIR}/${PROJECT_NAME_GLOBAL}/icons)
else ()
  set (CLIENT_ICONS_DEST1 ${CLIENT_ICONS_DEST})
  set (CLIENT_ICONS_DEST2 ${CLIENT_ICONS_DEST})
endif ()

  install (DIRECTORY icons/appl icons/emot icons/user DESTINATION ${CLIENT_ICONS_DEST2} PATTERN ".svn" EXCLUDE)
  install (FILES icons/icon_128x128.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/128x128/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_64x64.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/64x64/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_32x32.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/32x32/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_48x48.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/48x48/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_22x22.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/22x22/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_24x24.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/24x24/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_16x16.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/16x16/apps RENAME eiskaltdcpp.png)
  install (FILES ${TRANSLATIONS_BINARY} DESTINATION ${CLIENT_TRANSLATIONS_DEST})
if (APPLE)
  install (FILES icons/icon_128x128.png DESTINATION ${PROJECT_NAME_GLOBAL}.app/pixmaps/ RENAME eiskaltdcpp.png)
  install (FILES eiskaltdcpp-qt.desktop DESTINATION ${PROJECT_NAME_GLOBAL}.app/applications)
  install (FILES eiskaltdcpp-qt.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
  install (FILES default.rcc DESTINATION ${PROJECT_NAME_GLOBAL}.app/resources)
else (APPLE)
  install (FILES icons/icon_128x128.png DESTINATION ${SHARE_DIR}/pixmaps/ RENAME eiskaltdcpp.png)
  install (FILES default.rcc DESTINATION ${CLIENT_DATA_DIR}/resources)
  if (UNIX)
    #install (FILES default.rcc DESTINATION ${CLIENT_DATA_DIR}/resources)
    install (FILES eiskaltdcpp-qt.1.gz DESTINATION ${SHARE_DIR}/man/man1)
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

if( WIN32 AND NOT UNIX )
    set(CPACK_PACKAGE_EXECUTABLES "..\\\\EiskaltDC++" "EiskaltDC++")
endif()

include(CPack)
