#****************************************************************************
# Copyright © 2019 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2019-04-22.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
#****************************************************************************
cmake_minimum_required(VERSION 3.13)

## Defines the target SDL3::SDL3.

if (NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 INTERFACE IMPORTED)
    target_compile_options(SDL3::SDL3
        INTERFACE
        "SHELL:-s USE_SDL=3"
    )
    target_link_options(SDL3::SDL3
        INTERFACE
        "SHELL:-s USE_SDL=3"
    )
endif ()

set(SDL3_FOUND YES)
