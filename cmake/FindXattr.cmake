# - Try to find libattr library and headers
# Once done, this will define
#
#  XATTR_FOUND - system has libattr
#  XATTR_INCLUDE_DIRS - the libattr include directories
#  XATTR_LIBRARIES - link these to use libattr
 
FIND_PATH(XATTR_INCLUDE xattr.h
  ${XATTR_PREFIX}/include/attr
  /usr/include/attr
)

FIND_LIBRARY(XATTR_LIB
  NAMES
    attr
  PATHS
    /usr/lib
    ${XATTR_PREFIX}/lib
)

IF(XATTR_INCLUDE AND XATTR_LIB)
  SET(XATTR_FOUND TRUE)
  SET(XATTR_INCLUDE_DIRS ${XATTR_INCLUDE})
  SET(XATTR_LIBRARIES ${XATTR_LIB})
ELSE(XATTR_INCLUDE AND XATTR_LIB)
  SET(XATTR_FOUND FALSE)
  SET(XATTR_LIBRARIES "")
ENDIF(XATTR_INCLUDE AND XATTR_LIB)