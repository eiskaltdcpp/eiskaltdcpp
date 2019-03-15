cmake_minimum_required (VERSION 2.6.3)

if (CMAKE_PREFIX_PATH MATCHES "mxe" AND NOT STATIC)
    set (LIBS_TARGET install-dependencies)
    set (COMMON_LIBS_DIR "${CMAKE_PREFIX_PATH}/bin")
    set (QT_LIBS_DIR "${CMAKE_PREFIX_PATH}/qt5/bin")
    set (QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/qt5/plugins")

    set (COMMON_LIBS
            libaspell-15.dll
            libboost_system-mt.dll
            libbz2.dll
            libcrypto-1_1.dll
            libcrypto-1_1-x64.dll
            libdbus-1-3.dll
            libexpat-1.dll
            libfontconfig-1.dll
            libfreetype-6.dll
            libgcc_s_seh-1.dll
            libgcc_s_sjlj-1.dll
            libglib-2.0-0.dll
            libharfbuzz-0.dll
            libharfbuzz-icu-0.dll
            libiconv-2.dll
            libidn-12.dll
            libintl-8.dll
            libjpeg-9.dll
            libminiupnpc.dll
            libpcre-1.dll
            libpcre2-16-0.dll
            libpcrecpp-0.dll
            libpng16-16.dll
            libsqlite3-0.dll
            libssl-1_1.dll
            libssl-1_1-x64.dll
            libstdc++-6.dll
            libwinpthread-1.dll
            lua53.dll
            zlib1.dll
        )

    set (QT_LIBS
            Qt5Concurrent.dll
            Qt5Core.dll
            Qt5DBus.dll
            Qt5Gui.dll
            Qt5Multimedia.dll
            Qt5Network.dll
            Qt5Sql.dll
            Qt5Widgets.dll
            Qt5Xml.dll
        )

    set (QT_PLUGINS
            audio
            bearer
            generic
            iconengines
            imageformats
            mediaservice
            platforms
            platformthemes
            playlistformats
            printsupport
            sqldrivers
            styles
        )

    foreach (FILE ${COMMON_LIBS})
        if (EXISTS "${COMMON_LIBS_DIR}/${FILE}")
            list (APPEND LIBS_TO_INSTALL "${COMMON_LIBS_DIR}/${FILE}")
        endif ()
    endforeach ()

    foreach (FILE ${QT_LIBS})
        if (EXISTS "${QT_LIBS_DIR}/${FILE}")
            list (APPEND LIBS_TO_INSTALL "${QT_LIBS_DIR}/${FILE}")
        endif ()
    endforeach ()

    foreach (DIR ${QT_PLUGINS})
        if (EXISTS "${QT_PLUGINS_DIR}/${DIR}")
            list (APPEND DIRS_TO_INSTALL "${QT_PLUGINS_DIR}/${DIR}")
        endif ()
    endforeach ()

    install (FILES ${LIBS_TO_INSTALL} DESTINATION ${BINDIR})
    install (DIRECTORY ${DIRS_TO_INSTALL} DESTINATION ${BINDIR})
endif ()

