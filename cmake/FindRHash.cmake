# - Try to find the RHASH library and headers
# Once done this will define
#
#  RHASH_INCLUDE_DIR - the RHASH include directory
#  RHASH_LIBRARIES - link these to use RHASH

if(RHASH_INCLUDE_DIR AND RHASH_LIBRARY)
  set(RHASH_FIND_QUIETLY TRUE)
endif()

# Include dir
find_path(RHASH_INCLUDE_DIR NAMES rhash/rhash.h)

# Libraries
find_library(RHASH_LIBRARY NAMES rhash)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RHASH DEFAULT_MSG RHASH_LIBRARIES RHASH_INCLUDE_DIR)

MARK_AS_ADVANCED(RHASH_LIBRARIES RHASH_INCLUDE_DIR)
