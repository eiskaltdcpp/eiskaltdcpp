if (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
  # Already in cache, be silent
  set(PCRE_FIND_QUIETLY TRUE)
endif (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)

#find_path(PCRE_INCLUDE_DIR pcre.h
   #PATH_SUFFIXES pcre)
#find_library(PCRE_LIBRARY pcre)
#find_path(PCRE++_INCLUDE_DIR pcre++.h
   #PATH_SUFFIXES pcre++)
#find_library(PCRE++_LIBRARY pcre++)
find_program(PCRE_CONFIG pcre-config)
#find_program(PCRE++_CONFIG pcre++-config)
pkg_check_modules(PCRE pcrecpp)
execute_process (COMMAND ${PCRE_CONFIG} --libs-cpp
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                OUTPUT_VARIABLE PCRE_LIBRARY
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process (COMMAND ${PCRE_CONFIG} --cflags
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                OUTPUT_VARIABLE PCRE_INCLUDE_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

find_path(RE_INCLUDE_DIR pcre.h
   PATH_SUFFIXES pcre)
set (PCRE_INCLUDE_DIR ${RE_INCLUDE_DIR})
#find_library(PCRE_PCRE_LIBRARY pcre )
##execute_process (COMMAND ${PCRE++_CONFIG} --libs
                ##WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                ##OUTPUT_VARIABLE PCRE++_LIBRARY)
##execute_process (COMMAND ${PCRE_CONFIG} --cflags
                ##WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                ##OUTPUT_VARIABLE PCRE++_INCLUDE_DIR)
#message(STATUS "${PCRE_LIBRARIES}")
#message(STATUS "${PCRE_LIBRARY_DIRS}")
#message(STATUS "${PCRE_LDFLAGS}")
#message(STATUS "${PCRE_LDFLAGS_OTHER}")
#message(STATUS "${PCRE_INCLUDE_DIRS}")
#message(STATUS "${PCRE_CFLAGS}")
#message(STATUS "${PCRE_CFLAGS_OTHER}")
#message(STATUS "${PCRE_LIBRARY}")
#message(STATUS "${PCRE_INCLUDE_DIR}")
#message(STATUS "${RE_INCLUDE_DIR}")

#set(PCRE_INCLUDE_DIR ${PCRE_INCLUDE_DIRS})
#set (PCRE_LIBRARY ${PCRE_LDFLAGS})

if (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
    set (PCRE_FOUND TRUE)
endif (PCRE_INCLUDE_DIR AND PCRE_LIBRARY)

if (PCRE_FOUND)
  if (NOT PCRE_FIND_QUIETLY)
    message (STATUS "Found the pcre(cpp) libraries at ${PCRE_LIBRARY}")
    message (STATUS "Found the pcre(cpp) headers at ${PCRE_INCLUDE_DIR}")
  endif (NOT PCRE_FIND_QUIETLY)
else ()
    message (STATUS "Could not find pcre(cpp)")
endif ()

MARK_AS_ADVANCED(
  PCRE_INCLUDE_DIR
  PCRE_LIBRARY
)
#`pcre-config --cflags`
#`pcre-config --libs-cpp`
