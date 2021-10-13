# - Try to find GNU IDN2 library and headers
# Once done, this will define
#
#  IDN2_FOUND - system has IDN2
#  IDN2_INCLUDE_DIR - the IDN2 include directories (<idn2.h>)
#  IDN2_LIBRARIES - link these to use IDN2 (idn2_to_ascii_8z)

if (IDN2_INCLUDE_DIR AND IDN2_LIBRARIES)
  set(IDN2_FIND_QUIETLY TRUE)
endif (IDN2_INCLUDE_DIR AND IDN2_LIBRARIES)

# Include dir
find_path(IDN2_INCLUDE_DIR
  NAMES idn2.h
)

# Library
find_library(IDN2_LIBRARY
  NAMES idn2
)


# handle the QUIETLY and REQUIRED arguments and set IDN2_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IDN2 DEFAULT_MSG IDN2_LIBRARY IDN2_INCLUDE_DIR)

# If we successfully found the idn2 library then add the library to the
# IDN2_LIBRARIES cmake variable otherwise set IDN2_LIBRARIES to nothing.
IF(IDN2_FOUND)
   SET( IDN2_LIBRARIES ${IDN2_LIBRARY} )
ELSE(IDN2_FOUND)
   SET( IDN2_LIBRARIES )
ENDIF(IDN2_FOUND)


# Lastly make it so that the IDN2_LIBRARIES and IDN2_INCLUDE_DIR variables
# only show up under the advanced options in the gui cmake applications.
MARK_AS_ADVANCED( IDN2_LIBRARIES IDN2_INCLUDE_DIR IDN2_LIBRARY)

