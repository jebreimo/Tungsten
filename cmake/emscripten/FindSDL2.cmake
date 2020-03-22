#****************************************************************************
# Copyright © 2019 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2019-04-22.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
#****************************************************************************
cmake_minimum_required(VERSION 3.13)

## Defines the target SDL2::SDL2.

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

set(SDL2_FOUND YES)
