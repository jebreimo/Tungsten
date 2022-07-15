#****************************************************************************
# Copyright © 2020 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2020-03-22.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
#****************************************************************************
cmake_minimum_required(VERSION 3.13)

if (NOT TARGET OpenGL::GL)
    add_library(OpenGL::GL INTERFACE IMPORTED)
endif ()

set(OpenGL_FOUND YES)
