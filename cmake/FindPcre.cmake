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

if (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
    set (PCRE_FOUND TRUE)
else ()
    execute_process (COMMAND ${PCRE_CONFIG} --libs-cpp
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    OUTPUT_VARIABLE RE_LIBRARY
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    find_path(RE_INCLUDE_DIR pcrecpp.h)
    set (PCRE_INCLUDE_DIR ${RE_INCLUDE_DIR})
    set (PCRE_LIBRARY ${RE_LIBRARY})
endif (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)

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
