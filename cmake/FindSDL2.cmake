#****************************************************************************
# Copyright © 2019 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2019-04-22.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
#****************************************************************************

## Defines the target SDL2::SDL2.
##
## Components:
## - OpenGL: defines the target SDL2::OpenGL which makes OpenGL and GLEW
##   available to targets that link with it.

list(FIND SDL2_FIND_COMPONENTS OpenGL FIND_OPENGL)

if (EMSCRIPTEN)

    if (NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
        target_compile_options(SDL2::SDL2
            INTERFACE
                "SHELL:-s USE_SDL=2"
            )
        target_link_options(SDL2::SDL2
            INTERFACE
                "SHELL:-s USE_SDL=2"
            )
    endif ()

    if ((NOT TARGET SDL2::OpenGL) AND (${FIND_OPENGL} GREATER_EQUAL 0))
        add_library(SDL2::OpenGL INTERFACE IMPORTED)
        target_compile_options(SDL2::OpenGL
            INTERFACE
                "SHELL:-s FULL_ES2=1"
            )
    endif ()

    set(SDL2_FOUND 1)

else ()

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

    if ((TARGET SDL2::SDL2) AND (NOT TARGET SDL2::OpenGL) AND (${FIND_OPENGL} GREATER_EQUAL 0))
        if (SDL2_FIND_REQUIRED)
            find_package(GLEW REQUIRED)
            find_package(OpenGL REQUIRED)
        else ()
            find_package(GLEW)
            find_package(OpenGL)
        endif ()
        if (${GLEW_FOUND} AND ${OpenGL_FOUND})
            add_library(SDL2::OpenGL INTERFACE IMPORTED)
            target_link_libraries(SDL2::OpenGL
                INTERFACE
                    GLEW::GLEW
                    OpenGL::GL
                )
        endif ()
    endif ()
endif ()
