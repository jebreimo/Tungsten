# ===========================================================================
# Copyright © 2026 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2026-04-24.
#
# This file is distributed under the Zero-Clause BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(catch
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG "v3.5.3"
)
FetchContent_MakeAvailable(catch)

add_executable(TungstenTest
    test_FontUtilities.cpp
    test_TextUtilities.cpp
    text_Font.cpp
)

target_link_libraries(TungstenTest
    Tungsten::Tungsten
    Catch2::Catch2WithMain
)

Xyz_enable_all_warnings(TungstenTest)

add_test(NAME TungstenTest COMMAND TungstenTest)
