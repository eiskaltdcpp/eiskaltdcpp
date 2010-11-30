project (${PROJECT_NAME_GLOBAL}-gtk)

cmake_minimum_required (VERSION 2.6)

set (GTK2_DIR "${PROJECT_SOURCE_DIR}/cmake/")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/src gtk_srcs)
file (GLOB gtk_hdrs ${PROJECT_SOURCE_DIR}/src/*.hh)
file (GLOB gtk_glade ${PROJECT_SOURCE_DIR}/glade/*.glade)

set (GTK_SRCS @gtk_srcs@ PARENT_SCOPE)
set (GTK_HDRS @gtk_hdrs@ PARENT_SCOPE)
set (GTK_GLADE @gtk_glade@ PARENT_SCOPE)
set (GTHREAD "-lgthread-2.0")

include_directories (
        ${GTK2_INCLUDE_DIRS}
        ${LIBGNOME2_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/..
        ${Boost_INCLUDE_DIRS}
        )
list(APPEND LIBS
        ${X11_LIBRARIES}
        ${GTK2_LIBRARIES}
        ${LIBGNOME2_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ICONV_LIBRARIES}
        ${LIBNOTIFY}
        ${GTHREAD}
        )

if (LUA_SCRIPT)
  include_directories (${LUA_INCLUDE_DIR})
  list (APPEND LIBS ${LUA_LIBRARIES})
  set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/mainwindow.cc ${PROJECT_SOURCE_DIR}/src/hub.cc PROPERTY COMPILE_DEFINITIONS LUA_SCRIPT)
endif (LUA_SCRIPT)

add_executable (eiskaltdcpp-gtk MACOSX_BUNDLE WIN32
                ${gtk_srcs}
                )

if (USE_LIBNOTIFY)
    add_definitions (-DUSE_LIBNOTIFY)
endif (USE_LIBNOTIFY)

add_definitions (-DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}"
                -DCLIENT_SOUNDS_DIR="${CLIENT_SOUNDS_DEST}"
                -DCLIENT_TRANSLATIONS_DIR="${CLIENT_TRANSLATIONS_DEST}"
                -DCLIENT_ICONS_DIR="${CLIENT_DATA_DIR}/gtk/icons"
                )

if (eiskaltdcpp_WC_REVISION)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/wulfor.cc  ${PROJECT_SOURCE_DIR}/src/mainwindow.cc PROPERTY COMPILE_DEFINITIONS DCPP_REVISION="${eiskaltdcpp_WC_REVISION}")
endif ()

if (USE_MINIUPNP)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/settingsdialog.cc  PROPERTY COMPILE_DEFINITIONS USE_MINIUPNP )
endif()

if (HAVE_IFADDRS_H)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/WulforUtil.cc PROPERTY COMPILE_DEFINITIONS HAVE_IFADDRS_H )
endif (HAVE_IFADDRS_H)

if (FORCE_XDG)
   set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/WulforUtil.cc PROPERTY COMPILE_DEFINITIONS FORCE_XDG )
endif (FORCE_XDG)

if (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${PROJECT_NAME_GLOBAL}.app/resources")
else (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/gtk/resources")
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
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/eiskaltdcpp-gtk.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-gtk.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

  set_property (TARGET eiskaltdcpp-gtk PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-gtk")
  install (TARGETS eiskaltdcpp-gtk
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

  install (DIRECTORY glade/ DESTINATION "${CLIENT_DATA_DIR}/gtk/glade"
    PATTERN ".svn" EXCLUDE)
  install (DIRECTORY icons/ DESTINATION "${CLIENT_DATA_DIR}/gtk/icons"
    PATTERN ".svn" EXCLUDE)
  install (DIRECTORY themes/ DESTINATION "${CLIENT_DATA_DIR}/gtk/themes"
    PATTERN ".svn" EXCLUDE)

if (APPLE)
  install (FILES eiskaltdcpp-gtk.desktop DESTINATION ${PROJECT_NAME_GLOBAL}.app/applications)
  install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-gtk.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
else (APPLE)
  if (UNIX)
    install (FILES eiskaltdcpp-gtk.desktop DESTINATION ${SHARE_DIR}/applications)
    install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-gtk.1.gz DESTINATION ${SHARE_DIR}/man/man1)
  endif (UNIX)
endif (APPLE)
