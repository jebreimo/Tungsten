##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the Zero-Clause BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.15)

project(Tungsten VERSION 0.8.2)

set(CMAKE_CXX_STANDARD 23)
# Set TUNGSTEN_MASTER_PROJECT to ON if Xyz is the root level project.
if (NOT DEFINED TUNGSTEN_MASTER_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TUNGSTEN_MASTER_PROJECT ON)
    else ()
        set(TUNGSTEN_MASTER_PROJECT OFF)
    endif ()
endif ()

# Test option
option(TUNGSTEN_BUILD_TEST "Build tests" ${TUNGSTEN_MASTER_PROJECT})

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
    GIT_TAG v0.11.0)

FetchContent_Declare(yconvert
    GIT_REPOSITORY "https://github.com/jebreimo/Yconvert.git"
    GIT_TAG v0.4.1)

FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG v0.2.4)

FetchContent_MakeAvailable(argos cppembed xyz yconvert yimage)

if (NOT EMSCRIPTEN)
    find_package(Freetype REQUIRED)
endif ()

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL3 REQUIRED)

add_library(Tungsten STATIC
    include/Tungsten/Camera.hpp
    include/Tungsten/Detail/GenericBitmaskOperators.hpp
    include/Tungsten/DeviceInfo.hpp
    include/Tungsten/Environment.hpp
    include/Tungsten/Gl/DummyOglWrapper.hpp
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
    include/Tungsten/MeshDataBuilder.hpp
    include/Tungsten/MeshUtilities.hpp
    include/Tungsten/Render/Font.hpp
    include/Tungsten/Render/FontManager.hpp
    include/Tungsten/Render/FontManager.hpp
    include/Tungsten/Render/Lights.hpp
    include/Tungsten/Render/Materials.hpp
    include/Tungsten/Render/Renderable.hpp
    include/Tungsten/Render/ShaderManager.hpp
    include/Tungsten/Render/ShaderPreprocessor.hpp
    include/Tungsten/Render/ShaderProgramBuilder.hpp
    include/Tungsten/Render/ShaderPrograms/LightUniforms.hpp
    include/Tungsten/Render/ShaderPrograms/MaterialUniforms.hpp
    include/Tungsten/Render/ShaderPrograms/ShaderProgram.hpp
    include/Tungsten/Render/ShaderPrograms/SmoothShader.hpp
    include/Tungsten/Render/ShaderTools.hpp
    include/Tungsten/Render/TextItem.hpp
    include/Tungsten/Render/TextRenderer.hpp
    include/Tungsten/Render/VertexArrayObject.hpp
    include/Tungsten/Render/VertexArrayObjectBuilder.hpp
    include/Tungsten/Render/VertexAttributeDefinition.hpp
    include/Tungsten/Render/VertexAttributeType.hpp
    include/Tungsten/Sdl/SdlApplication.hpp
    include/Tungsten/Sdl/SdlDisplay.hpp
    include/Tungsten/Sdl/SdlGlContext.hpp
    include/Tungsten/Sdl/SdlGlParameters.hpp
    include/Tungsten/Sdl/SdlSession.hpp
    include/Tungsten/Sdl/WindowParameters.hpp
    include/Tungsten/Tungsten.hpp
    include/Tungsten/TungstenException.hpp
    include/Tungsten/Viewport.hpp
    include/Tungsten/YimageGl.hpp
    src/Tungsten/Camera.cpp
    src/Tungsten/Detail/BuddyAllocator.cpp
    src/Tungsten/Detail/BuddyAllocator.hpp
    src/Tungsten/DeviceInfo.cpp
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
    src/Tungsten/MeshUtilities.cpp
    src/Tungsten/Render/Font.cpp
    src/Tungsten/Render/FontManager.cpp
    src/Tungsten/Render/FontUtilities.cpp
    src/Tungsten/Render/FontUtilities.hpp
    src/Tungsten/Render/Materials.cpp
    src/Tungsten/Render/ShaderManager.cpp
    src/Tungsten/Render/ShaderPreprocessor.cpp
    src/Tungsten/Render/ShaderProgram.cpp
    src/Tungsten/Render/ShaderProgramBuilder.cpp
    src/Tungsten/Render/ShaderPrograms/LightUniforms.cpp
    src/Tungsten/Render/ShaderPrograms/MaterialUniforms.cpp
    src/Tungsten/Render/ShaderPrograms/SmoothShader.cpp
    src/Tungsten/Render/ShaderSources.cpp.in
    src/Tungsten/Render/ShaderSources.hpp
    src/Tungsten/Render/ShaderTools.cpp
    src/Tungsten/Render/TextItem.cpp
    src/Tungsten/Render/TextRenderer.cpp
    src/Tungsten/Render/TextRenderer/Monaco32.hpp
    src/Tungsten/Render/TextRenderer/Monaco64.hpp
    src/Tungsten/Render/TextRenderer/RenderTextShaderProgram.cpp
    src/Tungsten/Render/TextRenderer/RenderTextShaderProgram.hpp
    src/Tungsten/Render/TextUtilities.cpp
    src/Tungsten/Render/TextUtilities.hpp
    src/Tungsten/Render/VertexArrayObject.cpp
    src/Tungsten/Render/VertexArrayObjectBuilder.cpp
    src/Tungsten/Render/VertexAttributeType.cpp
    src/Tungsten/Sdl/CommandLine.cpp
    src/Tungsten/Sdl/CommandLine.hpp
    src/Tungsten/Sdl/SdlApplication.cpp
    src/Tungsten/Sdl/SdlDisplay.cpp
    src/Tungsten/Sdl/SdlGlContext.cpp
    src/Tungsten/Sdl/SdlGlParameters.cpp
    src/Tungsten/Sdl/SdlSession.cpp
    src/Tungsten/ExceptionHelpers.cpp
    src/Tungsten/Viewport.cpp
    src/Tungsten/YimageGl.cpp
    src/Tungsten/Neo/RenderItem.cpp
    src/Tungsten/Neo/RenderItem.hpp
    src/Tungsten/Neo/ResourceRef.hpp
    src/Tungsten/Neo/ResourceRefs.hpp
    src/Tungsten/Neo/Material.hpp
    src/Tungsten/Neo/Mesh.hpp
    src/Tungsten/Neo/BufferArena.cpp
    src/Tungsten/Neo/BufferArena.hpp
    src/Tungsten/Neo/ResourceManager.hpp
    src/Tungsten/Neo/SharedBuffer.hpp
    src/Tungsten/ExceptionHelpers.hpp
    src/Tungsten/Neo/ResourceManager.cpp
    src/Tungsten/Neo/LightData.cpp
    src/Tungsten/Neo/LightData.hpp
    src/Tungsten/Neo/RenderSnapshot.hpp
    src/Tungsten/Neo/GlStateCache.cpp
    src/Tungsten/Neo/GlStateCache.hpp
    src/Tungsten/Neo/UboBindings.hpp
    src/Tungsten/Neo/DeletionQueue.cpp
    src/Tungsten/Neo/DeletionQueue.hpp
    src/Tungsten/Neo/LayoutRegistry.cpp
    src/Tungsten/Neo/LayoutRegistry.hpp
    src/Tungsten/Neo/ShaderFamily.hpp
    src/Tungsten/Neo/ShaderProgram.hpp
    src/Tungsten/Neo/ShaderVariantKey.hpp
    src/Tungsten/Neo/Texture.hpp
    src/Tungsten/Neo/VaoCache.cpp
    src/Tungsten/Neo/VaoCache.hpp
    src/Tungsten/Neo/VertexAttribute.hpp
    src/Tungsten/Neo/VertexLayout.hpp
    src/Tungsten/Neo/VertexStream.hpp
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

if (NOT EMSCRIPTEN)
    target_link_libraries(Tungsten
        PUBLIC
            Freetype::Freetype
    )
endif ()

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
    src/Tungsten/Render/ShaderSources.cpp.in
)

##
## Unit tests
##

if (TUNGSTEN_BUILD_TEST)
    enable_testing()
    add_subdirectory(tests/TungstenTest)
endif ()

##
## Examples
##

if (TUNGSTEN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
