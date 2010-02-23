project (eiskaltdcpp)
cmake_minimum_required (VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include (${QT_USE_FILE})
set (HDRS   ArenaWidget.h
            Func.h
            HistoryInterface.h
            PMWindow.h
            PoolAlloc.h
            Version.h)
set (SRCS   Antispam.cpp
            AntiSpamFrame.cpp
            ArenaWidget.cpp
            DownloadQueue.cpp
            DownloadQueueModel.cpp
            FavoriteHubModel.cpp
            FavoriteHubs.cpp
            FavoriteUsers.cpp
            FileBrowserModel.cpp
            FinishedTransfers.cpp
            FinishedTransfersModel.cpp
            HashProgress.cpp
            HubFrame.cpp
            HubManager.cpp
            IPFilter.cpp
            IPFilterFrame.cpp
            IPFilterModel.cpp
            Magnet.cpp
            MainWindow.cpp
            Notification.cpp
            PMWindow.cpp
            QuickConnect.cpp
            SearchFrame.cpp
            SearchModel.cpp
            Settings.cpp
            SettingsConnection.cpp
            SettingsDownloads.cpp
            SettingsGUI.cpp
            SettingsNotification.cpp
            SettingsPersonal.cpp
            SettingsSharing.cpp
            ShareBrowser.cpp
            ShellCommandRunner.cpp
            SingleInstanceRunner.cpp
            ToolBar.cpp
            TransferView.cpp
            TransferViewModel.cpp
            UPnP.cpp
            UPnPMapper.cpp
            UserListModel.cpp
            WulforManager.cpp
            WulforSettings.cpp
            WulforUtil.cpp)
set (MOC_HDRS Antispam.h
            AntiSpamFrame.h
            DownloadQueue.h
            DownloadQueueModel.h
            FavoriteHubModel.h
            FavoriteHubs.h
            FavoriteUsers.h
            FileBrowserModel.h
            FinishedTransfers.h
            FinishedTransfersModel.h
            HashProgress.h
            HubFrame.h
            HubManager.h
            IPFilter.h
            IPFilterFrame.h
            IPFilterModel.h
            Magnet.h
            MainWindow.h
            Notification.h
            PMWindow.h
            Settings.h
            SettingsConnection.h
            SettingsDownloads.h
            SettingsGUI.h
            SettingsNotification.h
            SettingsPersonal.h
            SettingsSharing.h
            SingleInstanceRunner.h
            QuickConnect.h
            SearchFrame.h
            SearchModel.h
            ShareBrowser.h
            ShellCommandRunner.h
            ToolBar.h
            TransferView.h
            TransferViewModel.h
            UPnP.h
            UPnPMapper.h
            UserListModel.h
            WulforManager.h
            WulforSettings.h
            WulforUtil.h )
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
            ui/UIQuickConnect.ui
            ui/UISearchFrame.ui
            ui/UISettings.ui
            ui/UISettingsConnection.ui
            ui/UISettingsDownloads.ui
            ui/UISettingsGUI.ui
            ui/UISettingsNotification.ui
            ui/UISettingsPersonal.ui
            ui/UISettingsSharing.ui
            ui/UIShareBrowser.ui
            ui/UITransferView.ui)
set (LANGUAGES en
               ru)

set (COMPILED_TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/translations/eiskaltdcpp.en.qm
                           ${CMAKE_CURRENT_SOURCE_DIR}/translations/eiskaltdcpp.ru.qm)

#set (PO_FILES po/ru.po)

#set (MO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/po/en.mo
#              ${CMAKE_CURRENT_SOURCE_DIR}/po/ru.mo)

#GETTEXT_CREATE_TRANSLATIONS ( po/ru.po ALL ${PO_FILES})

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

add_custom_target (
        translations_qt
        COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} -ts ${TRANSLATIONS})
add_custom_command (
        TARGET translations_qt
        COMMAND ${QT_LRELEASE_EXECUTABLE} ${TRANSLATIONS})

include_directories (${QT_INCLUDES} ${UPNP_INCLUDE_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/..)
set (LIBS ${LIBS} ${QT_LIBRARIES} ${UPNP_LIBRARY})
add_executable (eiskaltdcpp WIN32 main.cpp
                ${M_SRCS}
                ${U_SRCS}
                ${SRCS}
                ${HDRS}
                ${MOC_HDRS}
                ${TRANSLATIONS}
                translations)
add_definitions (-I. -I.. -DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}" -DCLIENT_SOUNDS_DIR="${CLIENT_SOUNDS_DEST}" -DCLIENT_TRANSLATIONS_DIR="${CLIENT_TRANSLATIONS_DEST}" -DCLIENT_ICONS_DIR="${CLIENT_ICONS_DEST}" )

set_target_properties (eiskaltdcpp PROPERTIES LINK_FLAGS -Wl,--as-needed,--no-undefined)
target_link_libraries (eiskaltdcpp ${LIBS} dcpp)

  set_property (TARGET eiskaltdcpp PROPERTY OUTPUT_NAME ${PROJECT_NAME_GLOBAL})
  install (TARGETS eiskaltdcpp RUNTIME DESTINATION bin)
  install (DIRECTORY icons/appl icons/emot icons/user DESTINATION ${CLIENT_ICONS_DEST} PATTERN ".svn" EXCLUDE)
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
