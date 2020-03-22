#****************************************************************************
# Copyright © 2019 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2019-04-22.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
#****************************************************************************
cmake_minimum_required(VERSION 3.13)

## Defines the target SDL2::SDL2.
##
## Components:
## - OpenGL: defines the target SDL2::OpenGL which makes OpenGL and GLEW
##   available to targets that link with it.

list(FIND SDL2_FIND_COMPONENTS OpenGL FIND_OPENGL)

find_package(PkgConfig)
pkg_check_modules(PC_SDL2 QUIET sdl2)

find_path(SDL2_INCLUDE_DIR
    NAMES SDL.h
    PATHS ${PC_SDL2_INCLUDE_DIRS}
    PATH_SUFFIXES SDL2
    )

find_library(SDL2_LIBRARY
    NAMES SDL2 libSDL2
    PATHS ${PC_SDL2_LIBRARY_DIRS}
    )

set(SDL2_VERSION ${PC_SDL2_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_INCLUDE_DIR
    VERSION_VAR SDL2_VERSION
    )

mark_as_advanced(
    SDL2_FOUND
    SDL2_INCLUDE_DIR
    SDL2_LIBRARY
    SDL2_VERSION)

if (SDL2_FOUND AND NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(SDL2::SDL2
        PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        )
endif ()
