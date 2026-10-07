# ===========================================================================
# Copyright © 2026 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2026-10-06.
#
# This file is distributed under the Zero-Clause BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)

add_executable(coordsys
    coordsys.cpp
    Resources.hpp
)

target_link_libraries(coordsys
    PRIVATE
        Tungsten::Tungsten
)

target_embed_cpp_data(coordsys
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources.cpp.in
)
