project (eiskaltdcpp-gtk)

cmake_minimum_required (VERSION 2.6)

set (GTK2_DIR "${PROJECT_SOURCE_DIR}/cmake/")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/src gtk_srcs)
file (GLOB gtk_hdrs ${PROJECT_SOURCE_DIR} src/*.hh)
#message ("gtk_srcs:@gtk_srcs@ gtk_hdrs:@gtk_hdrs@")
set (GTK_SRCS @gtk_srcs@ PARENT_SCOPE)
set (GTK_HDRS @gtk_hdrs@ PARENT_SCOPE)

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
        "-lgthread-2.0"
        )


add_executable (eiskaltdcpp-gtk MACOSX_BUNDLE WIN32
                ${gtk_srcs}
                )
add_definitions (-DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}"
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

  install (DIRECTORY glade/ DESTINATION "${_DATADIR}/gtk/glade"
    PATTERN ".svn" EXCLUDE)
  install (DIRECTORY icons/ DESTINATION "${_DATADIR}/gtk/icons"
    PATTERN ".svn" EXCLUDE)

if (APPLE)
  install (FILES eiskaltdcpp-gtk.desktop DESTINATION ${PROJECT_NAME_GLOBAL}.app/applications)
  install (FILES eiskaltdcpp-gtk.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
else (APPLE)
  if (UNIX)
    install (FILES eiskaltdcpp-gtk.desktop DESTINATION ${SHARE_DIR}/applications)
    install (FILES eiskaltdcpp-gtk.1.gz DESTINATION ${SHARE_DIR}/man/man1)
  endif (UNIX)
endif (APPLE)
