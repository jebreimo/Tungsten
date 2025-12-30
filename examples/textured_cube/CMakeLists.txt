# ===========================================================================
# Copyright © 2025 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2025-12-03.
#
# This file is distributed under the Zero-Clause BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(ystring2
    GIT_REPOSITORY "https://github.com/jebreimo/Ystring2.git"
    GIT_TAG v2.3.1)

FetchContent_MakeAvailable(ystring2)

add_executable(textured_cube
    MeshItem.cpp
    MeshItem.hpp
    Resources.hpp.in
    TexturedCube.cpp
)

target_link_libraries(textured_cube
    PRIVATE
        Argos::Argos
        Tungsten::Tungsten
        Yconvert::Yconvert
        Ystring2::Ystring
)

target_embed_cpp_data(textured_cube
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources.hpp.in
)
