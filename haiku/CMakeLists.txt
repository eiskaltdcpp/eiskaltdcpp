project(haiku)

cmake_minimum_required (VERSION 2.6)

if (HAIKU_PKG)
    EXECUTE_PROCESS (
        COMMAND rc -o ${PROJECT_BINARY_DIR}/haiku.rsrc haiku.rdef
        COMMAND xres -o ${PROJECT_BINARY_DIR}/eiskaltdcpp-qt ${PROJECT_BINARY_DIR}/haiku.rsrc)
endif ()
