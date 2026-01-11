# ===========================================================================
# Copyright © 2024 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2024-11-09.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)

add_executable(fading_blob
    Shape2D.cpp
    Shape2D.hpp
    fading_blob.cpp
    SceneFader.cpp
    SceneFader.hpp
    Resources.hpp
)

target_link_libraries(fading_blob
    PRIVATE
        Tungsten::Tungsten
)

target_embed_cpp_data(fading_blob
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources.cpp.in
)
