project(haiku)

cmake_minimum_required (VERSION 2.6)

if (HAIKU_PKG)
    install (CODE "rc -o \"${PROJECT_BINARY_DIR}/haiku.rsrc\"
                         \"${PROJECT_SOURCE_DIR}/haiku/haiku.rdef\"")
    install (CODE "xres -o \"${PROJECT_BINARY_DIR}/eiskaltdcpp-qt/eiskaltdcpp-qt\"
                           \"${PROJECT_BINARY_DIR}/haiku.rsrc\"")
endif ()
