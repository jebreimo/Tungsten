##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.15)

project(Tungsten VERSION 0.8.2)

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
    GIT_TAG v0.9.5)

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
    include/Tungsten/Gl/GlBuffer.hpp
    include/Tungsten/Gl/GlFramebuffer.hpp
    include/Tungsten/Gl/GlHandle.hpp
    include/Tungsten/Gl/GlProgram.hpp
    include/Tungsten/Gl/GlRendering.hpp
    include/Tungsten/Gl/GlShader.hpp
    include/Tungsten/Gl/GlStateManagement.hpp
    include/Tungsten/Gl/GlTexture.hpp
    include/Tungsten/Gl/GlUniform.hpp
    include/Tungsten/Gl/GlVertexArray.hpp
    include/Tungsten/Gl/IOglWrapper.hpp
    include/Tungsten/GlContext.hpp
    include/Tungsten/GlParameters.hpp
    include/Tungsten/MeshUtilities.hpp
    include/Tungsten/SdlApplication.hpp
    include/Tungsten/SdlDisplay.hpp
    include/Tungsten/SdlSession.hpp
    include/Tungsten/ShaderManager.hpp
    include/Tungsten/ShaderPreprocessor.hpp
    include/Tungsten/ShaderProgramBuilder.hpp
    include/Tungsten/ShaderPrograms/GouraudShader.hpp
    include/Tungsten/ShaderPrograms/PhongShader.hpp
    include/Tungsten/ShaderPrograms/ShaderProgram.hpp
    include/Tungsten/ShaderPrograms/SmoothShader.hpp
    include/Tungsten/ShaderTools.hpp
    include/Tungsten/TextRenderer.hpp
    include/Tungsten/Tungsten.hpp
    include/Tungsten/TungstenException.hpp
    include/Tungsten/VertexArrayDataBuilder.hpp
    include/Tungsten/VertexArrayObject.hpp
    include/Tungsten/VertexArrayObjectBuilder.hpp
    include/Tungsten/Viewport.hpp
    include/Tungsten/WindowParameters.hpp
    include/Tungsten/YimageGl.hpp
    src/Tungsten/Camera.cpp
    src/Tungsten/CommandLine.cpp
    src/Tungsten/CommandLine.hpp
    src/Tungsten/DeviceInfo.cpp
    src/Tungsten/FontManager.cpp
    src/Tungsten/Gl/GlBuffer.cpp
    src/Tungsten/Gl/GlFramebuffer.cpp
    src/Tungsten/Gl/GlProgram.cpp
    src/Tungsten/Gl/GlRendering.cpp
    src/Tungsten/Gl/GlShader.cpp
    src/Tungsten/Gl/GlStateManagement.cpp
    src/Tungsten/Gl/GlTexture.cpp
    src/Tungsten/Gl/GlTypeConversion.cpp
    src/Tungsten/Gl/GlTypeConversion.hpp
    src/Tungsten/Gl/GlUniform.cpp
    src/Tungsten/Gl/GlVertexArray.cpp
    src/Tungsten/Gl/IOglWrapper.cpp
    src/Tungsten/Gl/OglWrapper.hpp
    src/Tungsten/Gl/TracingOglWrapper.hpp
    src/Tungsten/GlContext.cpp
    src/Tungsten/GlParameters.cpp
    src/Tungsten/MeshUtilities.cpp
    src/Tungsten/SdlApplication.cpp
    src/Tungsten/SdlDisplay.cpp
    src/Tungsten/SdlSession.cpp
    src/Tungsten/ShaderManager.cpp
    src/Tungsten/ShaderPreprocessor.cpp
    src/Tungsten/ShaderProgram.cpp
    src/Tungsten/ShaderProgramBuilder.cpp
    src/Tungsten/ShaderPrograms/GouraudShader.cpp
    src/Tungsten/ShaderPrograms/PhongShader.cpp
    src/Tungsten/ShaderPrograms/SmoothShader.cpp
    src/Tungsten/ShaderSources.cpp.in
    src/Tungsten/ShaderSources.hpp
    src/Tungsten/ShaderTools.cpp
    src/Tungsten/TextRenderer.cpp
    src/Tungsten/TextRenderer/Monaco32.hpp
    src/Tungsten/TextRenderer/Monaco64.hpp
    src/Tungsten/TextRenderer/RenderTextShaderProgram.cpp
    src/Tungsten/TextRenderer/RenderTextShaderProgram.hpp
    src/Tungsten/TungstenException.cpp
    src/Tungsten/VertexArrayObject.cpp
    src/Tungsten/VertexArrayObjectBuilder.cpp
    src/Tungsten/YimageGl.cpp
    include/Tungsten/Commands/Command.cpp
    include/Tungsten/Commands/Command.hpp
    include/Tungsten/Commands/DrawElementsCommand.cpp
    include/Tungsten/Commands/DrawElementsCommand.hpp
    include/Tungsten/Commands/CompositeCommand.cpp
    include/Tungsten/Commands/CompositeCommand.hpp
    include/Tungsten/ShaderPrograms2/ShaderProgram.hpp
    src/Tungsten/Viewport.cpp
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
