# - Try to find libgnome2 
# Find libgnome2 headers, libraries and the answer to all questions.
#
#  LIBGNOME2_FOUND               True if libgnome2 got found
#  LIBGNOME2_INCLUDEDIR          Location of libgnome2 headers 
#  LIBGNOME2_LIBRARIES           List of libaries to use libgnome2
#  LIBGNOME2_DEFINITIONS         Definitions to compile libgnome2 
#
# Copyright (c) 2007 Juha Tuomala <tuju@iki.fi>
# Copyright (c) 2007 Daniel Gollub <gollub@b1-systems.de>
# Copyright (c) 2007 Alban Browaeys <prahal@yahoo.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#








INCLUDE( FindPkgConfig )
# Take care about libgnome-2.0.pc settings
IF ( LibGnome2_FIND_REQUIRED )
  SET( _pkgconfig_REQUIRED "REQUIRED" )
ELSE ( LibGnome2_FIND_REQUIRED )
  SET( _pkgconfig_REQUIRED "" )
ENDIF ( LibGnome2_FIND_REQUIRED )

IF ( LIBGNOME2_MIN_VERSION )
	pkg_search_module( LIBGNOME2 ${_pkgconfig_REQUIRED} libgnome-2.0>=${LIBGNOME2_MIN_VERSION} )
ELSE ( LIBGNOME2_MIN_VERSION )
	pkg_search_module( LIBGNOME2 ${_pkgconfig_REQUIRED} libgnome-2.0 )
ENDIF ( LIBGNOME2_MIN_VERSION )


# Look for libgnome2 include dir and libraries w/o pkgconfig
IF ( NOT LIBGNOME2_FOUND AND NOT PKG_CONFIG_FOUND )
	FIND_PATH( _libgnome2_include_DIR libgnome/libgnome.h PATH_SUFFIXES libgnome-2.0 
		PATHS
		/opt/local/include/
		/sw/include/
		/usr/local/include/
		/usr/include/
	)
	FIND_LIBRARY( _libgnome2_link_DIR gnome-2 
		PATHS
		/opt/local/lib
		/sw/lib
		/usr/lib
		/usr/local/lib
		/usr/lib64
		/usr/local/lib64
		/opt/lib64
	)
	IF ( _libgnome2_include_DIR AND _libgnome2_link_DIR )
		SET ( _libgnome2_FOUND TRUE )
	ENDIF ( _libgnome2_include_DIR AND _libgnome2_link_DIR )


	IF ( _libgnome2_FOUND )
		SET ( LIBGNOME2_INCLUDE_DIRS ${_libgnome2_include_DIR} )
		SET ( LIBGNOME2_LIBRARIES ${_libgnome2_link_DIR} )
	ENDIF ( _libgnome2_FOUND )

	# Handle dependencies
	IF ( NOT BONOBO2_FOUND )
		FIND_PACKAGE( BONOBO2 REQUIRED)
		IF ( BONOBO2_FOUND )
			SET ( LIBGNOME2_INCLUDE_DIRS ${LIBGNOME2_INCLUDE_DIRS} ${BONOBO2_INCLUDE_DIRS} )
			SET ( LIBGNOME2_LIBRARIES ${LIBGNOME2_LIBRARIES} ${BONOBO2_LIBRARIES} )
		ENDIF ( BONOBO2_FOUND )
	ENDIF ( NOT BONOBO2_FOUND )
        IF ( NOT GLIB2_FOUND )
                FIND_PACKAGE( GLIB2 REQUIRED)

                IF ( GMODULE2_FOUND )
                        SET ( LIBGNOME2_INCLUDE_DIRS ${LIBGNOME2_INCLUDE_DIRS} ${GMODULE2_INCLUDE_DIR} )
                        SET ( LIBGNOME2_LIBRARIES ${LIBGNOME2_LIBRARIES} ${GMODULE2_LIBRARY} )
                ENDIF ( GMODULE2_FOUND )
                IF ( GLIB2_FOUND )
                        SET ( LIBGNOME2_INCLUDE_DIRS ${LIBGNOME2_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIR} ${GLIBCONFIG_INCLUDE_DIR} )
                        SET ( LIBGNOME2_LIBRARIES ${LIBGNOME2_LIBRARIES} ${GLIB2_LIBRARY} )
                ENDIF ( GLIB2_FOUND )
        ENDIF ( NOT GLIB2_FOUND )



	# Report results
	IF ( LIBGNOME2_LIBRARIES AND LIBGNOME2_INCLUDE_DIRS AND _libgnome2_FOUND )	
		SET( LIBGNOME2_FOUND 1 )
		IF ( NOT LibGnome2_FIND_QUIETLY )
			MESSAGE( STATUS "Found libgnome2: ${LIBGNOME2_LIBRARIES} ${LIBGNOME2_INCLUDE_DIRS}" )
		ENDIF ( NOT LibGnome2_FIND_QUIETLY )
	ELSE ( LIBGNOME2_LIBRARIES AND LIBGNOME2_INCLUDE_DIRS AND _libgnome2_FOUND )	
		IF ( LibGnome2_FIND_REQUIRED )
			MESSAGE( SEND_ERROR "Could NOT find libgnome2" )
		ELSE ( LibGnome2_FIND_REQUIRED )
			IF ( NOT LibGnome2_FIND_QUIETLY )
				MESSAGE( STATUS "Could NOT find libgnome2" )	
			ENDIF ( NOT LibGnome2_FIND_QUIETLY )
		ENDIF ( LibGnome2_FIND_REQUIRED )
	ENDIF ( LIBGNOME2_LIBRARIES AND LIBGNOME2_INCLUDE_DIRS AND _libgnome2_FOUND )	

ENDIF ( NOT LIBGNOME2_FOUND AND NOT PKG_CONFIG_FOUND )

# Hide advanced variables from CMake GUIs
MARK_AS_ADVANCED( LIBGNOME2_LIBRARIES LIBGNOME2_INCLUDE_DIRS )

