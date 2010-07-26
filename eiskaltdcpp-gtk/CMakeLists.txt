project (eiskaltdcpp-gtk)

cmake_minimum_required (VERSION 2.6)

set (GTK2_DIR "${PROJECT_SOURCE_DIR}/cmake/")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (HDRS
        src/bacon-message-connection.h
        src/favoriteusers.hh
        src/previewmenu.hh
        src/sound.hh
        src/bookentry.hh
        src/finishedtransfers.hh
        src/privatemessage.hh
        src/transfers.hh
        src/dialogentry.hh
        src/func.hh
        src/publichubs.hh
        src/treeview.hh
        src/downloadqueue.hh
        src/hashdialog.hh
        src/search.hh
        src/UserCommandMenu.hh
        src/emoticonsdialog.hh
        src/hub.hh
        src/searchspy.hh
        src/version.hh
        src/emoticons.hh
        src/mainwindow.hh
        src/settingsdialog.hh
        src/wulformanager.hh
        src/entry.hh
        src/message.hh
        src/settingsmanager.hh
        src/WulforUtil.hh
        src/favoritehubs.hh
        src/notify.hh
        src/sharebrowser.hh
    )
set (SRCS
        src/bacon-message-connection.c
        src/favoriteusers.cc
        src/publichubs.cc
        src/treeview.cc
        src/bookentry.cc
        src/finishedtransfers.cc
        src/search.cc
        src/UserCommandMenu.cc
        src/dialogentry.cc
        src/hashdialog.cc
        src/searchspy.cc
        src/wulfor.cc
        src/downloadqueue.cc
        src/hub.cc
        src/settingsdialog.cc
        src/wulformanager.cc
        src/emoticons.cc
        src/mainwindow.cc
        src/settingsmanager.cc
        src/WulforUtil.cc
        src/emoticonsdialog.cc
        src/notify.cc
        src/sharebrowser.cc
        src/entry.cc
        src/previewmenu.cc
        src/sound.cc
        src/favoritehubs.cc
        src/privatemessage.cc
        src/transfers.cc
    )


include_directories (
        ${GTK2_INCLUDE_DIRS}
        ${LIBGNOME2_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/..
        ${Boost_INCLUDE_DIRS}
        )
set (LIBS ${LIBS}
        ${GTK2_LIBRARIES}
        ${LIBGNOME2_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ICONV_LIBRARIES}
        "-lnotify"
        )


add_executable (eiskaltdcpp-gtk MACOSX_BUNDLE WIN32 src/wulfor.cc
                ${SRCS}
                ${HDRS}
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
  target_link_libraries (eiskaltdcpp-gtk ${LIBS} dcpp iconv intl)
else (APPLE)
  target_link_libraries (eiskaltdcpp-gtk ${LIBS} dcpp)
endif (APPLE)

if (UNIX)
 set (BINDIR bin)
 if (APPLE)
    set (BUNDLEDIR .)
 endif ()
elseif (WIN32)
  set (BINDIR .)
endif (UNIX)

if (UNIX)
  FIND_PROGRAM (GZIP_CMD gzip)
  EXECUTE_PROCESS (
                OUTPUT_FILE eiskaltdcpp-gtk.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-gtk.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

  set_property (TARGET eiskaltdcpp-gtk PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-gtk")
  install (TARGETS eiskaltdcpp-gtk
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})



if (APPLE)
  install (FILES eiskaltdcpp-gtk.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
elseif (UNIX AND NOT APPLE)
  install (FILES eiskaltdcpp-gtk.1.gz DESTINATION ${SHARE_DIR}/man/man1)
endif (APPLE)
