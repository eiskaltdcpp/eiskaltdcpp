project(data)

if (USE_QT OR USE_QT5 OR USE_GTK OR USE_GTK3)
    if (WITH_EXAMPLES)
        install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        install (FILES update_geoip DESTINATION ${CLIENT_DATA_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif (WITH_EXAMPLES)

    if (WITH_LUASCRIPTS AND LUA_SCRIPT)
        install (DIRECTORY luascripts/ DESTINATION ${CLIENT_DATA_DIR}/luascripts)
    endif (WITH_LUASCRIPTS AND LUA_SCRIPT)

    if (WITH_SOUNDS)
        install (DIRECTORY sounds/ DESTINATION ${CLIENT_DATA_DIR}/sounds)
    endif (WITH_SOUNDS)

    if (WITH_EMOTICONS)
        install (DIRECTORY emoticons/ DESTINATION ${CLIENT_DATA_DIR}/emoticons)
    endif (WITH_EMOTICONS)

    if (NOT HAIKU)
        if (UNIX AND NOT APPLE)
            set (CLIENT_ICONS_DEST1 ${SHARE_DIR}/icons)
        else (UNIX AND NOT APPLE)
            set (CLIENT_ICONS_DEST1 ${CLIENT_ICONS_DEST})
        endif (UNIX AND NOT APPLE)
    endif (NOT HAIKU)

    if (UNIX AND NOT APPLE AND NOT HAIKU)
        list(APPEND iconsize 16 22 24 32 48 64 128)
        foreach (isize ${iconsize})
            install (FILES icons/icon_${isize}x${isize}.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/${isize}x${isize}/apps RENAME eiskaltdcpp.png)
        endforeach (isize)
    endif (UNIX AND NOT APPLE AND NOT HAIKU)

    if (UNIX AND NOT APPLE AND NOT HAIKU)
        install (FILES icons/icon_128x128.png DESTINATION ${SHARE_DIR}/pixmaps/ RENAME eiskaltdcpp.png)
    endif ()
endif (USE_QT OR USE_QT5 OR USE_GTK OR USE_GTK3)

if (USE_ASPELL AND LOCAL_ASPELL_DATA)
    install (DIRECTORY aspell/ DESTINATION ${CLIENT_DATA_DIR}/aspell)
endif (USE_ASPELL AND LOCAL_ASPELL_DATA)

