project (eiskaltdcpp-wt)

cmake_minimum_required (VERSION 2.6)

set (Wt_DIR "${PROJECT_SOURCE_DIR}/cmake/")

find_package (Wt REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Wt_INCLUDE_DIR})

set (HDRS   DownloadQueue.h
	    DownloadQueueModel.h
	    SearchFrame.h
	    SearchModel.h
	    Utils.h)
set (SRCS   DownloadQueue.cpp
	    DownloadQueueModel.cpp
	    SearchFrame.cpp
	    SearchModel.cpp
	    Utils.cpp)


include_directories (${Wt_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}  ${PROJECT_SOURCE_DIR}/.. ${Boost_INCLUDE_DIRS})
set (LIBS ${LIBS} ${Wt_LIBRARIES} ${Boost_LIBRARIES} ${ICONV_LIBRARIES})


add_executable (eiskaltdcpp-wt MACOSX_BUNDLE WIN32 main.cpp
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

  set_property (TARGET eiskaltdcpp-wt PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-wt")
  install (TARGETS eiskaltdcpp-wt
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION . )