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
    test_BuddyAllocator.cpp
    test_FontUtilities.cpp
    test_GenerationalPool.cpp
    test_Renderer.cpp
    test_ResourceManager.cpp
    test_SceneGraph.cpp
    test_SnapshotBuilder.cpp
    test_TextUtilities.cpp
    text_Font.cpp
)

target_link_libraries(TungstenTest
    Tungsten::Tungsten
    Catch2::Catch2WithMain
)

# The tests exercise internal classes whose headers include other internal
# headers by their src-relative paths.
target_include_directories(TungstenTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

Xyz_enable_all_warnings(TungstenTest)

add_test(NAME TungstenTest COMMAND TungstenTest)
