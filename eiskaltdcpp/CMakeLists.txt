project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6)
if (FREE_SPACE_BAR_C)
add_subdirectory(gnulib)
endif (FREE_SPACE_BAR_C)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (QT_USE_QTNETWORK 1)
set (QT_USE_QTXML 1)
set (QT_USE_QTDBUS 1)

include (${QT_USE_FILE})
set (HDRS   ArenaWidget.h
            ArenaWidgetContainer.h
            EmoticonObject.h
            Func.h
            HistoryInterface.h
            PMWindow.h
            PoolAlloc.h
            Version.h)
set (SRCS   Antispam.cpp
            AntiSpamFrame.cpp
            ArenaWidget.cpp
            ArenaWidgetContainer.cpp
            CustomFontModel.cpp
            DownloadQueue.cpp
            DownloadQueueModel.cpp
            EmoticonDialog.cpp
            EmoticonFactory.cpp
            FavoriteHubModel.cpp
            FavoriteHubs.cpp
            FavoriteUsers.cpp
            FavoriteUsersModel.cpp
            FileBrowserModel.cpp
            FinishedTransfers.cpp
            FinishedTransfersModel.cpp
            HashProgress.cpp
            HubFrame.cpp
            HubManager.cpp
            IPFilter.cpp
            IPFilterFrame.cpp
            IPFilterModel.cpp
            LineEdit.cpp
            Magnet.cpp
            MainWindow.cpp
            Notification.cpp
            PMWindow.cpp
            PublicHubs.cpp
            PublicHubsList.cpp
            PublicHubModel.cpp
            QuickConnect.cpp
            SearchFrame.cpp
            SearchModel.cpp
            Settings.cpp
            SettingsConnection.cpp
            SettingsDownloads.cpp
            SettingsGUI.cpp
            SettingsLog.cpp
            SettingsNotification.cpp
            SettingsPersonal.cpp
            SettingsSharing.cpp
            SettingsUC.cpp
            ShareBrowser.cpp
            ShellCommandRunner.cpp
            SideBar.cpp
            SingleInstanceRunner.cpp
            SpyFrame.cpp
            SpyModel.cpp
            ToolBar.cpp
            TransferView.cpp
            TransferViewModel.cpp
            UCModel.cpp
            UPnP.cpp
            UPnPMapper.cpp
            UserListModel.cpp
            WulforSettings.cpp
            WulforUtil.cpp
            ChatEdit.cpp)
set (MOC_HDRS Antispam.h
            AntiSpamFrame.h
            CustomFontModel.h
            DownloadQueue.h
            DownloadQueueModel.h
            EmoticonDialog.h
            EmoticonFactory.h
            FavoriteHubModel.h
            FavoriteHubs.h
            FavoriteUsers.h
            FavoriteUsersModel.h
            FileBrowserModel.h
            FinishedTransfers.h
            FinishedTransfersModel.h
            HashProgress.h
            HubFrame.h
            HubManager.h
            IPFilter.h
            IPFilterFrame.h
            IPFilterModel.h
            LineEdit.h
            Magnet.h
            MainWindow.h
            Notification.h
            PMWindow.h
            PublicHubs.h
            PublicHubsList.h
            PublicHubModel.h
            Settings.h
            SettingsConnection.h
            SettingsDownloads.h
            SettingsGUI.h
            SettingsLog.h
            SettingsNotification.h
            SettingsPersonal.h
            SettingsSharing.h
            SettingsUC.h
            SideBar.h
            SingleInstanceRunner.h
            QuickConnect.h
            SearchFrame.h
            SearchModel.h
            ShareBrowser.h
            ShellCommandRunner.h
            SpyFrame.h
            SpyModel.h
            ToolBar.h
            TransferView.h
            TransferViewModel.h
            UCModel.h
            UPnP.h
            UPnPMapper.h
            UserListModel.h
            WulforSettings.h
            WulforUtil.h
            ChatEdit.h)
set (UI_FORMS ui/UIAbout.ui
            ui/UIAntiSpam.ui
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
            )
option(LANGUAGES "Using translations")
if (NOT linguas)
set (LANGUAGES en ru be hu fr pl sr ua CACHE STRING "Using translations" FORCE)
else (NOT linguas)
string(REGEX MATCHALL [a-zA-Z]+
         linguas1 ${linguas})
#message(STATUS ${linguas1})
set (LANGUAGES ${linguas1} CACHE STRING "Using translations" FORCE)
endif (NOT linguas)

if (USE_ASPELL)
  set (SRCS ${SRCS} SpellCheck.cpp)
  set (MOC_HDRS ${MOC_HDRS} SpellCheck.h)
endif (USE_ASPELL)
foreach (LANGUAGE ${LANGUAGES})
set (COMPILED_TRANSLATIONS ${COMPILED_TRANSLATIONS} ${CMAKE_CURRENT_SOURCE_DIR}/translations/eiskaltdcpp.${LANGUAGE}.qm)
#message (STATUS ${COMPILED_TRANSLATIONS})
endforeach()
#set (PO_FILES po/ru.po)
#set (PO_FILES po/be.po)

#set (MO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/po/en.mo
#              ${CMAKE_CURRENT_SOURCE_DIR}/po/be.mo
#              ${CMAKE_CURRENT_SOURCE_DIR}/po/ru.mo)

#GETTEXT_CREATE_TRANSLATIONS ( po/ru.po ALL ${PO_FILES})
#GETTEXT_CREATE_TRANSLATIONS ( po/be.po ALL ${PO_FILES})

#Create moc files
QT4_WRAP_CPP (M_SRCS ${MOC_HDRS})
#Create .h files from ui files
QT4_WRAP_UI (U_SRCS ${UI_FORMS})
#Create translations
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/translations/eiskaltdcpp.${LANGUAGE}.ts)
        set (QM ${CMAKE_CURRENT_SOURCE_DIR}/translations/eiskaltdcpp.${LANGUAGE}.qm)
        set (TRANSLATIONS ${TRANSLATIONS} ${TS})
        set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
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
                OUTPUT_FILE eiskaltdcpp.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

if (UNIX)
  EXECUTE_PROCESS (
    COMMAND ${QT_RCC_EXECUTABLE} -binary  ${DEFAULT_QRC_FILE} -o default.rcc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  )
endif (UNIX)

include_directories (${QT_INCLUDES} ${UPNP_INCLUDE_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/.. ${Boost_INCLUDE_DIRS})
set (LIBS ${LIBS} ${QT_LIBRARIES} ${UPNP_LIBRARY} ${Boost_LIBRARIES} ${ICONV_LIBRARIES})

if (FREE_SPACE_BAR AND NOT FREE_SPACE_BAR_C)
add_definitions (-DFREE_SPACE_BAR)
set (LIBS ${LIBS} ${Boost_LIBRARIES})
endif (FREE_SPACE_BAR AND NOT FREE_SPACE_BAR_C)
if (FREE_SPACE_BAR_C AND NOT FREE_SPACE_BAR)
add_definitions (-DFREE_SPACE_BAR_C)
set (LIBS ${LIBS} gnu)
endif (FREE_SPACE_BAR_C AND NOT FREE_SPACE_BAR)

if (USE_ASPELL)
  add_definitions (-DUSE_ASPELL ${ASPELL_DEFINITIONS})
  include_directories (${ASPELL_INCLUDE_DIR})
  set (LIBS ${LIBS} ${ASPELL_LIBRARIES})
endif (USE_ASPELL)

if (APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE icon_128x128.icns)
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(SRCS ${SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns)
endif (APPLE)

add_executable (eiskaltdcpp MACOSX_BUNDLE WIN32 main.cpp
                ${M_SRCS}
                ${U_SRCS}
                ${SRCS}
                ${HDRS}
                ${MOC_HDRS}
                ${TRANSLATIONS}
                translations
                )
add_definitions (-I. -I.. -DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}"
                          -DCLIENT_SOUNDS_DIR="${CLIENT_SOUNDS_DEST}"
                          -DCLIENT_TRANSLATIONS_DIR="${CLIENT_TRANSLATIONS_DEST}"
                          -DCLIENT_ICONS_DIR="${CLIENT_ICONS_DEST}"
                )

if (APPLE)
  add_definitions(        -DCLIENT_RES_DIR="${PROJECT_NAME_GLOBAL}.app/resources")
else (APPLE)
  add_definitions(        -DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/resources")
endif (APPLE)

if (APPLE)
target_link_libraries (eiskaltdcpp ${LIBS} dcpp iconv intl upnp ixml)
else (APPLE)
target_link_libraries (eiskaltdcpp ${LIBS} dcpp)
endif (APPLE)

  set_property (TARGET eiskaltdcpp PROPERTY OUTPUT_NAME ${PROJECT_NAME_GLOBAL})
  install (TARGETS eiskaltdcpp
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION . )

if (NOT APPLE)

  install (DIRECTORY icons/appl icons/emot icons/user DESTINATION ${CLIENT_ICONS_DEST} PATTERN ".svn" EXCLUDE)
  install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE PATTERN ".svn" EXCLUDE )
  install (FILES icons/icon_128x128.png DESTINATION share/icons/hicolor/128x128/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_64x64.png DESTINATION share/icons/hicolor/64x64/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_32x32.png DESTINATION share/icons/hicolor/32x32/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_48x48.png DESTINATION share/icons/hicolor/48x48/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_22x22.png DESTINATION share/icons/hicolor/22x22/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_24x24.png DESTINATION share/icons/hicolor/24x24/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_16x16.png DESTINATION share/icons/hicolor/16x16/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_128x128.png DESTINATION share/pixmaps/ RENAME eiskaltdcpp.png)
  install (FILES ../eiskaltdcpp.desktop DESTINATION share/applications)
  install (FILES ${TRANSLATIONS_BINARY} DESTINATION ${CLIENT_TRANSLATIONS_DEST})
  install (FILES eiskaltdcpp.1.gz DESTINATION share/man/man1)
  install (FILES default.rcc DESTINATION ${CLIENT_DATA_DIR}/resources)

endif (NOT APPLE)

if (APPLE)

  INCLUDE( ${QT_USE_FILE} )

  set(plugin_dest_dir ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
  set(qtconf_dest_dir ${PROJECT_NAME_GLOBAL}.app/Contents/Resources)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME_GLOBAL}.app")
  set(DIRS "\${QT_LIBRARY_DIRS}" "\${PC_UPNP_LIBRARY_DIRS}" "\${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
  set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/ValknutVolumeIcon.icns)
  set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128x128.icns)
  set(CPACK_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
  if (NOT SVNVERSION OR SVNVERSION STREQUAL exported)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${VERSION}-stable")
  else (SVNVERSION)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_GLOBAL}-${VERSION}-${SVNVERSION}")
  endif (NOT SVNVERSION OR SVNVERSION STREQUAL exported)
  set (MACOSX_BUNDLE_INFO_STRING "DC and ADC client")
  set (MACOSX_BUNDLE_ICON_FILE icon_128x128.icns)

# doesn’t work this way :(
#  set (MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
  set_target_properties(eiskaltdcpp PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)

  set (MACOSX_BUNDLE_GUI_IDENTIFIER "http://code.google.com/p/eiskaltdc/")
  set (MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_NAME_GLOBAL} ${VERSION})
  set (MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME_GLOBAL})
  set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
  set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
  set (MACOSX_BUNDLE_COPYRIGH "Copyright 2010 Andrey Karlov")

  install (DIRECTORY icons/appl icons/emot icons/user DESTINATION ${CLIENT_ICONS_DEST} PATTERN ".svn" EXCLUDE)
  install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE PATTERN ".svn" EXCLUDE)
  install (FILES icons/icon_128x128.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/128x128/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_64x64.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/64x64/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_32x32.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/32x32/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_48x48.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/48x48/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_22x22.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/22x22/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_24x24.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/24x24/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_16x16.png DESTINATION ${CLIENT_ICONS_DEST}/hicolor/16x16/apps RENAME eiskaltdcpp.png)
  install (FILES icons/icon_128x128.png DESTINATION ${PROJECT_NAME_GLOBAL}.app/pixmaps/ RENAME eiskaltdcpp.png)
  install (FILES ../eiskaltdcpp.desktop DESTINATION ${PROJECT_NAME_GLOBAL}.app/applications)
  install (FILES ${TRANSLATIONS_BINARY} DESTINATION ${CLIENT_TRANSLATIONS_DEST})
  install (FILES eiskaltdcpp.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
  install (FILES default.rcc DESTINATION ${PROJECT_NAME_GLOBAL}.app/resources)

  install (DIRECTORY ${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib DESTINATION ${PROJECT_NAME_GLOBAL}.app/Contents/Resources)

  INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")")

  INSTALL(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")")
    set(CPACK_BINARY_DRAGNDROP ON)
    include(CPack)

endif (APPLE)
