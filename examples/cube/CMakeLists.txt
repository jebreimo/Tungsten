# ===========================================================================
# Copyright © 2025 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2025-12-03.
#
# This file is distributed under the Zero-Clause BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)

add_executable(cube
    Cube.cpp
    MeshItem.cpp
    MeshItem.hpp
)

target_link_libraries(cube
    PRIVATE
        Tungsten::Tungsten
        Yconvert::Yconvert
)
