project(${PROJECT_NAME_GLOBAL}-cli)
cmake_minimum_required(VERSION 2.6)

if (WIN32 OR HAIKU)
    set (BINDIR .)
elseif (UNIX)
    set (BINDIR bin)
    if (APPLE)
        set (BUNDLEDIR .)
    endif ()
endif (WIN32 OR HAIKU)

if (USE_CLI_XMLRPC)
    install (FILES cli-xmlrpc.pl DESTINATION ${BINDIR} RENAME ${PROJECT_NAME}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    install (FILES cli-xmlrpc-config.pl DESTINATION ${CLIENT_DATA_DIR}/cli
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    if (UNIX)
        execute_process (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-xmlrpc.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE ${PROJECT_NAME}-xmlrpc.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    endif (UNIX)

    if (APPLE)
        install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-xmlrpc.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
    elseif (UNIX AND NOT APPLE)
        install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-xmlrpc.1.gz DESTINATION ${SHARE_DIR}/man/man1)
    endif (APPLE)
endif ()

if (USE_CLI_JSONRPC)
    install (FILES cli-jsonrpc.pl DESTINATION ${BINDIR} RENAME ${PROJECT_NAME}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    install (FILES cli-jsonrpc-config.pl DESTINATION ${CLIENT_DATA_DIR}/cli
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    if (UNIX)
        execute_process (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-jsonrpc.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE ${PROJECT_NAME}-jsonrpc.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    endif (UNIX)

    if (APPLE)
        install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-jsonrpc.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
    elseif (UNIX AND NOT APPLE)
        install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-jsonrpc.1.gz DESTINATION ${SHARE_DIR}/man/man1)
    endif (APPLE)
endif ()

