project(data)

if (USE_QT OR USE_GTK)
    if (WITH_EXAMPLES)
        install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        PATTERN ".svn" EXCLUDE )
        install (FILES update_geoip DESTINATION ${CLIENT_DATA_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif (WITH_EXAMPLES)

    if (WITH_LUASCRIPTS AND LUA_SCRIPT)
        install (DIRECTORY luascripts/ DESTINATION ${CLIENT_DATA_DIR}/luascripts
        PATTERN ".svn" EXCLUDE )
    endif (WITH_LUASCRIPTS AND LUA_SCRIPT)

    if (WITH_SOUNDS)
        install (DIRECTORY sounds/ DESTINATION ${CLIENT_DATA_DIR}/sounds
        PATTERN ".svn" EXCLUDE )
    endif (WITH_SOUNDS)

    if (WITH_EMOTICONS)
        install (DIRECTORY emoticons/ DESTINATION ${CLIENT_DATA_DIR}/emoticons
        PATTERN ".svn" EXCLUDE)
    endif (WITH_EMOTICONS)

list(APPEND iconsize 16 22 24 32 48 64 128)
foreach (isize ${iconsize})
  if (NOT HAIKU)
    install (FILES icons/icon_${isize}x${isize}.png DESTINATION ${CLIENT_ICONS_DEST1}/hicolor/${isize}x${isize}/apps RENAME eiskaltdcpp.png)
  endif (NOT HAIKU)
endforeach (isize)
if (APPLE)
  install (FILES icons/icon_128x128.png DESTINATION ${PROJECT_NAME_GLOBAL}.app/pixmaps/ RENAME eiskaltdcpp.png)
else (APPLE)
  if (NOT HAIKU)
    install (FILES icons/icon_128x128.png DESTINATION ${SHARE_DIR}/pixmaps/ RENAME eiskaltdcpp.png)
  endif (NOT HAIKU)
endif (APPLE)
endif (USE_QT OR USE_GTK)
