project (eiskaltdcpp-wt)

cmake_minimum_required (VERSION 2.6)

set (Wt_DIR "${PROJECT_SOURCE_DIR}/cmake/")

find_package (Wt REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Wt_INCLUDE_DIR})

set (HDRS
	    src/DownloadQueue.h
	    src/DownloadQueueModel.h
	    src/SearchFrame.h
	    src/SearchModel.h
	    src/Utils.h
	)
set (SRCS
	    src/DownloadQueue.cpp
	    src/DownloadQueueModel.cpp
	    src/SearchFrame.cpp
	    src/SearchModel.cpp
	    src/Utils.cpp
	)


include_directories (${Wt_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}  ${PROJECT_SOURCE_DIR}/.. ${Boost_INCLUDE_DIRS})
set (LIBS ${LIBS} ${Wt_LIBRARIES} ${Boost_LIBRARIES} ${ICONV_LIBRARIES})


add_executable (eiskaltdcpp-wt MACOSX_BUNDLE WIN32 src/main.cpp
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
  target_link_libraries (eiskaltdcpp-wt ${LIBS} dcpp iconv intl)
else (APPLE)
  target_link_libraries (eiskaltdcpp-wt ${LIBS} dcpp)
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
                OUTPUT_FILE eiskaltdcpp-wt.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-wt.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

  set_property (TARGET eiskaltdcpp-wt PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-wt")
  install (TARGETS eiskaltdcpp-wt
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

  install (DIRECTORY ext/ DESTINATION ${CLIENT_DATA_DIR}/wt/ext
    PATTERN ".svn" EXCLUDE)
  install (DIRECTORY resources/ DESTINATION ${CLIENT_DATA_DIR}/wt/resources
    PATTERN "*.sh"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    PATTERN ".svn" EXCLUDE)
  install (FILES eiskaltdcpp.css DESTINATION ${CLIENT_DATA_DIR}/wt/)
  install (FILES eiskaltdcpp-wt_run.sh DESTINATION ${CLIENT_DATA_DIR}/wt/
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

if (APPLE)
  install (FILES eiskaltdcpp-wt.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
else (UNIX AND NOT APPLE)
  install (FILES eiskaltdcpp-wt.1.gz DESTINATION ${SHARE_DIR}/man/man1)
endif (APPLE)
