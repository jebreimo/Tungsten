# ===========================================================================
# Copyright © 2023 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2023-12-23.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
# ===========================================================================
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(yconvert
    GIT_REPOSITORY "https://github.com/jebreimo/Yconvert.git"
    GIT_TAG v0.4.1)

FetchContent_MakeAvailable(yconvert)

add_executable(touch_events
    touch_events.cpp
    RingBuffer.hpp
)

target_link_libraries(touch_events
    PRIVATE
        Tungsten::Tungsten
        Yconvert::Yconvert
)

if (EMSCRIPTEN)
    target_link_options(touch_events
        PRIVATE
            -sALLOW_MEMORY_GROWTH=1
            -sEXPORTED_FUNCTIONS=['_main']
            -sEXPORTED_RUNTIME_METHODS=['ccall']
            -sFORCE_FILESYSTEM=1
    )
    set(EMSCRIPTEN_TARGET_NAME touch_events)
    configure_file(emscripten/index.html.in index.html)
    configure_file(emscripten/index.css index.css)
endif ()
