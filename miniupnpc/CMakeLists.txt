project(miniupnpc)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(${PROJECT_SOURCE_DIR} miniupnpc_SRCS)
add_definitions (-DSTATICLIB)

if (WIN32)
  set (WIN_VERSION 0x501) #я не знаю на что это влияет
  add_definitions( -DWIN32 -D_WIN32_WINNT=${WIN_VERSION} -D_WIN32_IE=${WIN_VERSION} -DWINVER=${WIN_VERSION} -DWIN32_LEAN_AND_MEAN)
elseif (APPLE)
  add_definitions (-DMACOSX -D_DARWIN_C_SOURCE)
endif (WIN32)
# actual target:
add_library(miniupnpc STATIC ${miniupnpc_SRCS})

set_property(TARGET miniupnpc PROPERTY COMPILE_FLAGS -fPIC)

if (WIN32)
  target_link_libraries (miniupnpc "ws2_32" "iphlpapi")
endif (WIN32)
