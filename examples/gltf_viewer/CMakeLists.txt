# ===========================================================================
# Copyright © 2026 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2026-09-22.
#
# This file is distributed under the Zero-Clause BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)

add_executable(gltf_viewer
    gltf_viewer.cpp
)

target_link_libraries(gltf_viewer
    PRIVATE
        Argos::Argos
        Tungsten::Tungsten
)
