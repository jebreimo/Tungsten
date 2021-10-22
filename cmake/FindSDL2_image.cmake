##****************************************************************************
## Copyright © 2021 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2021-10-17.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.13)

## Defines the target SDL2::SDL_image.
##
## Components:
## - OpenGL: defines the target SDL2::OpenGL which makes OpenGL and GLEW
##   available to targets that link with it.

find_package(PkgConfig)
pkg_check_modules(PC_SDL2 QUIET sdl2)

find_path(SDL2_image_INCLUDE_DIR
    NAMES SDL_image.h
    PATHS ${PC_SDL2_INCLUDE_DIRS}
    PATH_SUFFIXES SDL2
    )

find_library(SDL2_image_LIBRARY
    NAMES SDL2_image libSDL2_image
    PATHS ${PC_SDL2_LIBRARY_DIRS}
    )

if (SDL2_image_INCLUDE_DIR)
    file(READ "${SDL2_image_INCLUDE_DIR}/SDL_image.h" HEADER_FILE_CONTENTS)

    string(REGEX MATCH "SDL_IMAGE_MAJOR_VERSION +([0-9]+)" _ ${HEADER_FILE_CONTENTS})
    set(VERSION_MAJOR ${CMAKE_MATCH_1})

    string(REGEX MATCH "SDL_IMAGE_MINOR_VERSION +([0-9]+)" _ ${HEADER_FILE_CONTENTS})
    set(VERSION_MINOR ${CMAKE_MATCH_1})

    string(REGEX MATCH "SDL_IMAGE_PATCHLEVEL +([0-9]+)" _ ${HEADER_FILE_CONTENTS})
    set(VERSION_PATCH ${CMAKE_MATCH_1})

    set(SDL2_image_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_image
    REQUIRED_VARS SDL2_image_INCLUDE_DIR SDL2_image_LIBRARY
    VERSION_VAR SDL2_image_VERSION
    )

mark_as_advanced(
    SDL2_image_FOUND
    SDL2_image_INCLUDE_DIR
    SDL2_image_LIBRARY
    SDL2_image_VERSION)

if (SDL2_image_FOUND AND NOT TARGET SDL2_image::SDL2_image)
    add_library(SDL2_image::SDL2_image SHARED IMPORTED)
    set_target_properties(SDL2_image::SDL2_image
        PROPERTIES
            IMPORTED_LOCATION "${SDL2_image_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_image_INCLUDE_DIR}"
        )
endif ()
