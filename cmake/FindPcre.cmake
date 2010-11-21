if (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
  # Already in cache, be silent
  set(PCREPP_FIND_QUIETLY TRUE)
endif ()

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
    else ()
        execute_process (COMMAND ${PCRE_CONFIG} --libs-cpp
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        OUTPUT_VARIABLE RE_LIBRARY
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
                        set (PCRE_LIBRARY ${RE_LIBRARY})
    endif()
endif ()

if (NOT PCRE_INCLUDE_DIR)
    find_path(RE_INCLUDE_DIR pcrecpp.h)
    set (PCRE_INCLUDE_DIR ${RE_INCLUDE_DIR})
endif()

if (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
    set (PCRE_FOUND TRUE)
endif (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)

if (PCRE_FOUND)
  if (NOT PCREPP_FIND_QUIETLY)
    message (STATUS "Found the pcre(cpp) libraries at ${PCRE_LIBRARY}")
    message (STATUS "Found the pcre(cpp) headers at ${PCRE_INCLUDE_DIR}")
  endif ()
else ()
    message (STATUS "Could not find pcre(cpp)")
endif ()

MARK_AS_ADVANCED(
  PCRE_INCLUDE_DIR
  PCRE_LIBRARY
)
