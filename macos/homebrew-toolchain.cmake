if(DEFINED ENV{HOMEBREW})
    set(HOMEBREW "$ENV{HOMEBREW}")
else()
    execute_process(COMMAND brew --prefix
                    OUTPUT_VARIABLE HOMEBREW
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    if(NOT HOMEBREW)
        if(EXISTS "/opt/homebrew")
            set(HOMEBREW "/opt/homebrew")
        else()
            set(HOMEBREW "/usr/local")
        endif()
    endif()
endif()

if(DEFINED ENV{OSX_ARCHITECTURES})
    set(OSX_ARCHITECTURES "$ENV{OSX_ARCHITECTURES}")
else()
    execute_process(COMMAND uname -m
                    OUTPUT_VARIABLE OSX_ARCHITECTURES
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(DEFINED ENV{OSX_DEPLOYMENT_TARGET})
    set(OSX_DEPLOYMENT_TARGET "$ENV{OSX_DEPLOYMENT_TARGET}")
else()
    if(OSX_ARCHITECTURES MATCHES "arm64")
        set(OSX_DEPLOYMENT_TARGET "11.0")
    else()
        set(OSX_DEPLOYMENT_TARGET "10.15")
    endif()
endif()

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${HOMEBREW}")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${HOMEBREW}/opt/gettext")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${HOMEBREW}/opt/openssl@1.1")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${HOMEBREW}/opt/openssl")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${HOMEBREW}/opt/qt@5")

set(CMAKE_C_COMPILER   "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_OSX_ARCHITECTURES "${OSX_ARCHITECTURES}"
    CACHE STRING "CMAKE_OSX_ARCHITECTURES")
set(CMAKE_OSX_DEPLOYMENT_TARGET "${OSX_DEPLOYMENT_TARGET}"
    CACHE STRING "CMAKE_OSX_DEPLOYMENT_TARGET")

set(CMAKE_INSTALL_PREFIX "${HOMEBREW}" CACHE PATH "Installation Prefix")
#set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug|Release|RelWithDebInfo|MinSizeRel")
