##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.13)
project(Tungsten)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
if (NOT TARGET Xyz::Xyz)
    find_package(Xyz REQUIRED)
endif()

set(PUBLIC_HEADERS
    include/Tungsten/GlBuffer.hpp
    include/Tungsten/GlContext.hpp
    include/Tungsten/GlError.hpp
    include/Tungsten/GlHandle.hpp
    include/Tungsten/GlProgram.hpp
    include/Tungsten/GlTextures.hpp
    include/Tungsten/Tungsten.hpp
    include/Tungsten/GlVertices.hpp
    include/Tungsten/SdlApplication.hpp
    include/Tungsten/ShaderTools.hpp
    include/Tungsten/Uniform.hpp
    include/Tungsten/WindowParameters.hpp)

set(SOURCE_FILES
    src/Tungsten/GlBuffer.cpp
    src/Tungsten/GlContext.cpp
    src/Tungsten/GlError.cpp
    src/Tungsten/GlProgram.cpp
    src/Tungsten/GlTextures.cpp
    src/Tungsten/GlVertices.cpp
    src/Tungsten/SdlApplication.cpp
    src/Tungsten/SdlSession.cpp
    src/Tungsten/SdlSession.hpp
    src/Tungsten/ShaderTools.cpp
    src/Tungsten/Uniform.cpp
    src/Tungsten/WindowParameters.cpp)

add_library(Tungsten STATIC
    ${PUBLIC_HEADERS}
    ${SOURCE_FILES}
    )

target_link_libraries(Tungsten
    PUBLIC
        GLEW::GLEW
        SDL2::SDL2
        Xyz::Xyz
    )

add_library(Tungsten::Tungsten ALIAS Tungsten)

target_include_directories(Tungsten
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
