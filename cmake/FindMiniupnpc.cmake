find_path(MINIUPNP_INCLUDE_DIR miniupnpc/miniupnpc.h
   PATH_SUFFIXES miniupnpc)
find_library(MINIUPNP_LIBRARY miniupnpc)

if (MINIUPNP_INCLUDE_DIR AND MINIUPNP_LIBRARY)
    set (MINIUPNP_FOUND TRUE)
endif ()

if (MINIUPNP_FOUND)
    message (STATUS "Found the miniupnpc libraries at ${MINIUPNP_LIBRARY}")
    message (STATUS "Found the miniupnpc headers at ${MINIUPNP_INCLUDE_DIR}")
else ()
    message (STATUS "Could not find upnp")
endif ()

MARK_AS_ADVANCED(
  MINIUPNP_INCLUDE_DIR
  MINIUPNP_LIBRARY
)
