##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.15)

project(Tungsten VERSION 0.3.3)

set(CMAKE_CXX_STANDARD 20)

# Set TUNGSTEN_MASTER_PROJECT to ON if Xyz is the root level project.
if (NOT DEFINED TUNGSTEN_MASTER_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TUNGSTEN_MASTER_PROJECT ON)
    else ()
        set(TUNGSTEN_MASTER_PROJECT OFF)
    endif ()
endif ()

# Install option
option(TUNGSTEN_INSTALL "Generate the install target" ${TUNGSTEN_MASTER_PROJECT})

option(TUNGSTEN_BUILD_EXAMPLES "Build the examples" ${TUNGSTEN_MASTER_PROJECT})

if (EMSCRIPTEN)
    list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten)
else ()
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
endif ()

include(FetchContent)

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG v1.2.1)

FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
    GIT_TAG v0.2.91)

FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG v0.1.7)

FetchContent_MakeAvailable(argos xyz yimage)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_library(Tungsten STATIC
    include/Tungsten/ArrayBuffer.hpp
    include/Tungsten/ArrayBufferBuilder.hpp
    include/Tungsten/GlBuffer.hpp
    include/Tungsten/GlContext.hpp
    include/Tungsten/GlHandle.hpp
    include/Tungsten/GlParameters.hpp
    include/Tungsten/GlProgram.hpp
    include/Tungsten/GlShader.hpp
    include/Tungsten/GlTextures.hpp
    include/Tungsten/GlVertexArray.hpp
    include/Tungsten/GlVertices.hpp
    include/Tungsten/SdlApplication.hpp
    include/Tungsten/ShaderProgramBuilder.hpp
    include/Tungsten/ShaderTools.hpp
    include/Tungsten/Tungsten.hpp
    include/Tungsten/TungstenException.hpp
    include/Tungsten/Uniform.hpp
    include/Tungsten/WindowParameters.hpp
    src/Tungsten/CommandLine.cpp
    src/Tungsten/CommandLine.hpp
    src/Tungsten/GlBuffer.cpp
    src/Tungsten/GlContext.cpp
    src/Tungsten/GlParameters.cpp
    src/Tungsten/GlProgram.cpp
    src/Tungsten/GlShader.cpp
    src/Tungsten/GlTextures.cpp
    src/Tungsten/GlVertexArray.cpp
    src/Tungsten/GlVertices.cpp
    src/Tungsten/SdlApplication.cpp
    src/Tungsten/SdlSession.hpp
    src/Tungsten/SdlSession.cpp
    src/Tungsten/ShaderProgramBuilder.cpp
    src/Tungsten/ShaderTools.cpp
    src/Tungsten/TungstenException.cpp
    src/Tungsten/Uniform.cpp
    include/Tungsten/Font.hpp
    src/Tungsten/TextRenderer.cpp
    include/Tungsten/TextRenderer.hpp
    include/Tungsten/YimageGl.hpp
    src/Tungsten/YimageGl.cpp
    src/Tungsten/details/RenderTextShaderProgram.cpp
    src/Tungsten/details/RenderTextShaderProgram.hpp
    src/Tungsten/FontManager.cpp
    include/Tungsten/FontManager.hpp
    include/Tungsten/FontManager.hpp
    src/Tungsten/details/Monaco64.hpp
    include/Tungsten/VertexArray.hpp
    include/Tungsten/Buffer.hpp
)

target_link_libraries(Tungsten
    PUBLIC
        GLEW::GLEW
        OpenGL::GL
        SDL2::SDL2
        Xyz::Xyz
        Yimage::Yimage
    PRIVATE
        Argos
)

add_library(Tungsten::Tungsten ALIAS Tungsten)

target_include_directories(Tungsten
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake)

include(TungstenTargetEmbedShaders)

tungsten_target_embed_shaders(Tungsten
    FILES
        src/Tungsten/shaders/RenderText-frag.glsl
        src/Tungsten/shaders/RenderText-vert.glsl
)

if (TUNGSTEN_BUILD_EXAMPLES)
    add_subdirectory(examples/show_text)
endif ()
