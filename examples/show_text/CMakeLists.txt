# ===========================================================================
# Copyright © 2023 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2023-09-05.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

add_executable(show_text
    show_text.cpp
)

target_link_libraries(show_text
    PRIVATE
        Tungsten::Tungsten
)
