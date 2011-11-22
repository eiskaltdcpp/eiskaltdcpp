project (${PROJECT_NAME_GLOBAL}-gtk)

cmake_minimum_required (VERSION 2.6)

set (GTK2_DIR "${PROJECT_SOURCE_DIR}/cmake/")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/src gtk_srcs)
file (GLOB gtk_hdrs ${PROJECT_SOURCE_DIR}/src/*.hh)
file (GLOB gtk_ui ${PROJECT_SOURCE_DIR}/ui/*.ui)

set (GTK_SRCS @gtk_srcs@ PARENT_SCOPE)
set (GTK_HDRS @gtk_hdrs@ PARENT_SCOPE)
set (GTK_UI   @gtk_ui@   PARENT_SCOPE)
set (GTHREAD "-lgthread-2.0")

if (USE_GTK)
    set(GTK_INCLUDE_DIRS ${GTK2_INCLUDE_DIRS})
    set(GTK_LIBRARIES ${GTK2_LIBRARIES})
elseif (USE_GTK3)
    add_definitions(-DGSEAL_ENABLE)
    #add_definitions(-DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED)
    set(GTK_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS})
    set(GTK_LIBRARIES ${GTK3_LIBRARIES})
endif(USE_GTK)

include_directories (
        ${GTK_INCLUDE_DIRS}
        ${LIBGNOME2_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/..
        ${Boost_INCLUDE_DIRS}
        )
list(APPEND LIBS
        ${X11_LIBRARIES}
        ${GTK_LIBRARIES}
        ${LIBGNOME2_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ICONV_LIBRARIES}
        ${LIBNOTIFY}
        ${GTHREAD}
        )

if (LUA_SCRIPT)
  include_directories (${LUA_INCLUDE_DIR})
  list (APPEND LIBS ${LUA_LIBRARIES})
endif (LUA_SCRIPT)

add_executable (${PROJECT_NAME} MACOSX_BUNDLE WIN32
                ${gtk_srcs}
                )

if (USE_LIBNOTIFY)
    add_definitions (-DUSE_LIBNOTIFY)
endif (USE_LIBNOTIFY)

if (FREE_SPACE_BAR_C)
  add_definitions (-DFREE_SPACE_BAR_C)
endif (FREE_SPACE_BAR_C)

if (CHECK_GTK_DEPRECATED)
    add_definitions(-DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED)
endif (CHECK_GTK_DEPRECATED)

remove_definitions(-DBUILDING_DCPP)

add_definitions (-DCLIENT_DATA_DIR="${CLIENT_DATA_DIR}"
                -DCLIENT_SOUNDS_DIR="${CLIENT_SOUNDS_DEST}"
                -DCLIENT_TRANSLATIONS_DIR="${CLIENT_TRANSLATIONS_DEST}"
                -DCLIENT_ICONS_DIR="${CLIENT_DATA_DIR}/gtk/icons"
                )

if (WITH_DHT)
  add_definitions ( -DWITH_DHT )
endif (WITH_DHT)

if (USE_MINIUPNP)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/settingsdialog.cc  PROPERTY COMPILE_DEFINITIONS USE_MINIUPNP )
endif()

if (FORCE_XDG)
   set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/WulforUtil.cc PROPERTY COMPILE_DEFINITIONS FORCE_XDG APPEND)
endif (FORCE_XDG)

if (HAVE_IFADDRS_H)
  set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/WulforUtil.cc PROPERTY COMPILE_DEFINITIONS HAVE_IFADDRS_H APPEND)
endif (HAVE_IFADDRS_H)

if (ENABLE_STACKTRACE)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/wulfor.cc PROPERTY COMPILE_DEFINITIONS ENABLE_STACKTRACE)
endif (ENABLE_STACKTRACE)

if (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${PROJECT_NAME_GLOBAL}.app/resources")
else (APPLE)
  add_definitions(-DCLIENT_RES_DIR="${CLIENT_DATA_DIR}/gtk/resources")
endif (APPLE)

if (APPLE)
  target_link_libraries (${PROJECT_NAME} ${LIBS} dcpp iconv intl)
else (APPLE)
  target_link_libraries (${PROJECT_NAME} ${LIBS} dcpp)
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
  EXECUTE_PROCESS (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE ${PROJECT_NAME}.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

  set_property (TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}")
  install (TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

  install (DIRECTORY ui/ DESTINATION "${CLIENT_DATA_DIR}/gtk/ui"
    PATTERN ".svn" EXCLUDE)
  install (DIRECTORY icons/ DESTINATION "${CLIENT_DATA_DIR}/gtk/icons"
    PATTERN ".svn" EXCLUDE)
  install (DIRECTORY themes/ DESTINATION "${CLIENT_DATA_DIR}/gtk/themes"
    PATTERN ".svn" EXCLUDE)

if (APPLE)
  install (FILES ${PROJECT_NAME}.desktop DESTINATION ${PROJECT_NAME_GLOBAL}.app/applications)
  install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
else (APPLE)
  if (UNIX)
    install (FILES ${PROJECT_NAME}.desktop DESTINATION ${SHARE_DIR}/applications)
    install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz DESTINATION ${SHARE_DIR}/man/man1)
  endif (UNIX)
endif (APPLE)

foreach(LANG ${LANGUAGES})
    install(FILES
        ${PROJECT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/${PROJECT_NAME}.mo
            DESTINATION ${LOCALE_DIR}/${LANG}/LC_MESSAGES)
endforeach(LANG ${LANGUAGES})
