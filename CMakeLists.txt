##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.13)
project(Tungsten)

set(CMAKE_CXX_STANDARD 17)

if (EMSCRIPTEN)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten)
else ()
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
endif ()

include(FetchContent)
FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git")
FetchContent_GetProperties(xyz)
if (NOT xyz_POPULATED)
    FetchContent_Populate(xyz)
    add_subdirectory(${xyz_SOURCE_DIR} ${xyz_BINARY_DIR})
endif ()

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_library(Tungsten STATIC
    include/Tungsten/GlBuffer.hpp
    include/Tungsten/GlContext.hpp
    include/Tungsten/GlError.hpp
    include/Tungsten/GlHandle.hpp
    include/Tungsten/GlProgram.hpp
    include/Tungsten/GlShader.hpp
    include/Tungsten/GlTextures.hpp
    include/Tungsten/GlVertexArray.hpp
    include/Tungsten/GlVertices.hpp
    include/Tungsten/SdlApplication.hpp
    include/Tungsten/ShaderTools.hpp
    include/Tungsten/Shapes.hpp
    include/Tungsten/Tungsten.hpp
    include/Tungsten/Uniform.hpp
    include/Tungsten/WindowParameters.hpp
    include/Tungsten/Shapes/AddCube.hpp
    include/Tungsten/Shapes/AddRectangle.hpp
    include/Tungsten/Shapes/AddRegularPolygon.hpp
    include/Tungsten/Shapes/AddStarPolygon.hpp
    include/Tungsten/Shapes/ArrayBufferBuilder.hpp
    include/Tungsten/Shapes/ArrayBufferRowBuilder.hpp
    src/Tungsten/GlBuffer.cpp
    src/Tungsten/GlContext.cpp
    src/Tungsten/GlError.cpp
    src/Tungsten/GlProgram.cpp
    src/Tungsten/GlShader.cpp
    src/Tungsten/GlTextures.cpp
    src/Tungsten/GlVertexArray.cpp
    src/Tungsten/GlVertices.cpp
    src/Tungsten/SdlApplication.cpp
    src/Tungsten/SdlSession.cpp
    src/Tungsten/SdlSession.hpp
    src/Tungsten/ShaderTools.cpp
    src/Tungsten/Uniform.cpp
    src/Tungsten/WindowParameters.cpp
    src/Tungsten/Shapes/AddCube.cpp
    src/Tungsten/Shapes/AddRectangle.cpp
    src/Tungsten/Shapes/AddRegularPolygon.cpp
    src/Tungsten/Shapes/AddStarPolygon.cpp
    src/Tungsten/Shapes/ArrayBufferBuilder.cpp
    src/Tungsten/Shapes/ArrayBufferRowBuilder.cpp
    )

target_link_libraries(Tungsten
    PUBLIC
        GLEW::GLEW
        OpenGL::GL
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
