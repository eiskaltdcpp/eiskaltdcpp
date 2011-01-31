# - Try to find READLINE
# Once done this will define
#
#  READLINE_FOUND - system has READLINE
#  READLINE_INCLUDE_DIR - the READLINE include directory
#  READLINE_LIBRARIES - Link these to use READLINE
#  READLINE_NEED_PREFIX - this is set if the functions are prefixed with BZ2_

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(READLINE_INCLUDE_DIR readline/readline.h )
FIND_LIBRARY(READLINE_LIBRARIES NAMES readline history READLINE )

# handle the QUIETLY and REQUIRED arguments and set READLINE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(READLINE DEFAULT_MSG READLINE_LIBRARIES READLINE_INCLUDE_DIR)

IF (READLINE_FOUND)
   INCLUDE(CheckLibraryExists)
   CHECK_LIBRARY_EXISTS(${READLINE_LIBRARIES} rl_initialize "" READLINE_NEED_PREFIX)
ENDIF (READLINE_FOUND)

MARK_AS_ADVANCED(READLINE_INCLUDE_DIR READLINE_LIBRARIES)
