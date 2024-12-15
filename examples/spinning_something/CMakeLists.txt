# ===========================================================================
# Copyright © 2024 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2024-11-09.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)

add_executable(spinning_something
    Shape2D.cpp
    Shape2D.hpp
    Something.cpp
    SceneFader.cpp
    SceneFader.hpp
)

target_link_libraries(spinning_something
    PRIVATE
        Tungsten::Tungsten
)
