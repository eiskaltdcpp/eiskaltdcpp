project(nasdc)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/..)
aux_source_directory(${PROJECT_SOURCE_DIR} nasdc_SRCS)
list (REMOVE_ITEM nasdc_SRCS  ${PROJECT_SOURCE_DIR}/main.cpp)
# actual target:
add_executable (nasdc MACOSX_BUNDLE WIN32
		#main.cpp
		${nasdc_SRCS}
                )
target_link_libraries (nasdc dcpp pthread)
if (UNIX)
 set (BINDIR bin)
 if (APPLE)
    set (BUNDLEDIR .)
 endif ()
elseif (WIN32)
  set (BINDIR .)
endif (UNIX)

set_property (TARGET nasdc PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-daemon")
  install (TARGETS nasdc
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})
