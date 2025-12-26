##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.15)

project(Tungsten VERSION 0.8.0)

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
    GIT_TAG v1.7.2)

FetchContent_Declare(cppembed
    GIT_REPOSITORY "https://github.com/jebreimo/cppembed.git"
    GIT_TAG "v0.1.10"
)

FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
    GIT_TAG v0.9.4)

FetchContent_Declare(yconvert
    GIT_REPOSITORY "https://github.com/jebreimo/Yconvert.git"
    GIT_TAG v0.4.1)

FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG v0.2.4)

FetchContent_MakeAvailable(argos cppembed xyz yconvert yimage)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL3 REQUIRED)

add_library(Tungsten STATIC
    include/Tungsten/Buffer.hpp
    include/Tungsten/Camera.hpp
    include/Tungsten/Detail/GenericBitmaskOperators.hpp
    include/Tungsten/DeviceInfo.hpp
    include/Tungsten/DummyOglWrapper.hpp
    include/Tungsten/Environment.hpp
    include/Tungsten/Font.hpp
    include/Tungsten/FontManager.hpp
    include/Tungsten/FontManager.hpp
    include/Tungsten/GlBuffer.hpp
    include/Tungsten/GlContext.hpp
    include/Tungsten/GlFramebuffer.hpp
    include/Tungsten/GlHandle.hpp
    include/Tungsten/GlParameters.hpp
    include/Tungsten/GlProgram.hpp
    include/Tungsten/GlRendering.hpp
    include/Tungsten/GlShader.hpp
    include/Tungsten/GlStateManagement.hpp
    include/Tungsten/GlTexture.hpp
    include/Tungsten/GlUniform.hpp
    include/Tungsten/GlVertexArray.hpp
    include/Tungsten/GlVertices.hpp
    include/Tungsten/ShaderPrograms/GouraudShader.hpp
    include/Tungsten/IOglWrapper.hpp
    include/Tungsten/MeshShapes.hpp
    include/Tungsten/ShaderPrograms/PhongShader.hpp
    include/Tungsten/SdlApplication.hpp
    include/Tungsten/SdlDisplay.hpp
    include/Tungsten/SdlSession.hpp
    include/Tungsten/ShaderManager.hpp
    include/Tungsten/ShaderPreprocessor.hpp
    include/Tungsten/ShaderProgram.hpp
    include/Tungsten/ShaderProgramBuilder.hpp
    include/Tungsten/ShaderTools.hpp
    include/Tungsten/ShaderPrograms/SmoothMeshShader.hpp
    include/Tungsten/TextRenderer.hpp
    include/Tungsten/Tungsten.hpp
    include/Tungsten/TungstenException.hpp
    include/Tungsten/VertexArrayDataBuilder.hpp
    include/Tungsten/VertexArrayObject.hpp
    include/Tungsten/VertexArrayObjectBuilder.hpp
    include/Tungsten/WindowParameters.hpp
    include/Tungsten/YimageGl.hpp
    src/Tungsten/CommandLine.cpp
    src/Tungsten/CommandLine.hpp
    src/Tungsten/DeviceInfo.cpp
    src/Tungsten/FontManager.cpp
    src/Tungsten/GlBuffer.cpp
    src/Tungsten/GlContext.cpp
    src/Tungsten/GlFramebuffer.cpp
    src/Tungsten/GlParameters.cpp
    src/Tungsten/GlProgram.cpp
    src/Tungsten/GlRendering.cpp
    src/Tungsten/GlShader.cpp
    src/Tungsten/GlStateManagement.cpp
    src/Tungsten/GlTexture.cpp
    src/Tungsten/GlTypeConversion.cpp
    src/Tungsten/GlTypeConversion.hpp
    src/Tungsten/GlUniform.cpp
    src/Tungsten/GlVertexArray.cpp
    src/Tungsten/GlVertices.cpp
    src/Tungsten/IOglWrapper.cpp
    src/Tungsten/MeshShapes.cpp
    src/Tungsten/OglWrapper.hpp
    src/Tungsten/SdlApplication.cpp
    src/Tungsten/SdlDisplay.cpp
    src/Tungsten/SdlSession.cpp
    src/Tungsten/ShaderManager.cpp
    src/Tungsten/ShaderPreprocessor.cpp
    src/Tungsten/ShaderProgram.cpp
    src/Tungsten/ShaderProgramBuilder.cpp
    src/Tungsten/ShaderPrograms/GouraudShader.cpp
    src/Tungsten/ShaderPrograms/PhongShader.cpp
    src/Tungsten/ShaderSources.cpp.in
    src/Tungsten/ShaderSources.hpp
    src/Tungsten/ShaderTools.cpp
    src/Tungsten/ShaderPrograms/SmoothMeshShader.cpp
    src/Tungsten/TextRenderer.cpp
    src/Tungsten/TextRenderer/Monaco32.hpp
    src/Tungsten/TextRenderer/Monaco64.hpp
    src/Tungsten/TextRenderer/RenderTextShaderProgram.cpp
    src/Tungsten/TextRenderer/RenderTextShaderProgram.hpp
    src/Tungsten/TracingOglWrapper.hpp
    src/Tungsten/TungstenException.cpp
    src/Tungsten/VertexArrayObject.cpp
    src/Tungsten/VertexArrayObjectBuilder.cpp
    src/Tungsten/YimageGl.cpp
    src/Tungsten/ShaderPrograms/TexturedPhongShader.cpp
    src/Tungsten/ShaderPrograms/TexturedSmoothMeshShader.cpp
)

target_link_libraries(Tungsten
    PUBLIC
        GLEW::GLEW
        OpenGL::GL
        SDL3::SDL3
        Xyz::Xyz
        Yconvert::Yconvert
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

list(APPEND CMAKE_MODULE_PATH ${cppembed_SOURCE_DIR}/cmake)
include(TargetEmbedCppData)
target_embed_cpp_data(Tungsten
    FILES
        src/Tungsten/ShaderSources.cpp.in
)

if (TUNGSTEN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
