find_program(PCRE_CONFIG pcre-config)
#`pcre-config --cflags`
#`pcre-config --libs-cpp`
#pkg_check_modules(PCRE libpcre libpcrecpp libpcreposix)
pkg_check_modules(PCRE libpcrecpp)
set (PCRE_INCLUDE_DIR ${PCRE_INCLUDE_DIRS})
set (PCRE_LIBRARY ${PCRE_LDFLAGS})
if (NOT PCRE_LIBRARY)
    execute_process (COMMAND ${PCRE_CONFIG} --version
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    OUTPUT_VARIABLE RE_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (RE_VERSION VERSION_LESS 8.02)
        find_library(RECPP_LIBRARY NAMES pcrecpp libpcrecpp)
        find_library(REC_LIBRARY NAMES pcre libpcre)
        set (PCRE_LIBRARY ${REC_LIBRARY} ${RECPP_LIBRARY})
        set (REC_LIBRARY) # unset
        set (RECPP_LIBRARY) # unset
    else ()
        execute_process (COMMAND ${PCRE_CONFIG} --libs-cpp
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        OUTPUT_VARIABLE RE_LIBRARY
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        set (PCRE_LIBRARY ${RE_LIBRARY})
        set (RE_LIBRARY) # unset
    endif()
    set (RE_VERSION) # unset
endif ()

if (NOT PCRE_INCLUDE_DIR)
    find_path(RE_INCLUDE_DIR pcrecpp.h)
    set (PCRE_INCLUDE_DIR ${RE_INCLUDE_DIR})
    set (RE_INCLUDE_DIR) # unset
endif()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS("Pcre(cpp)" DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)

MARK_AS_ADVANCED(
  PCRE_INCLUDE_DIR
  PCRE_LIBRARY
)
