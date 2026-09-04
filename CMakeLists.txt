##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the Zero-Clause BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.23)

project(Tungsten VERSION 0.8.2)

set(CMAKE_CXX_STANDARD 23)

# Set TUNGSTEN_MASTER_PROJECT to ON if Tungsten is the root level project.
if (NOT DEFINED TUNGSTEN_MASTER_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TUNGSTEN_MASTER_PROJECT ON)
        message("Using Tungsten ${PROJECT_VERSION}")
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

# Note: on macOS the arm64-osx-* CMake presets set CMAKE_IGNORE_PREFIX_PATH so
# CMake never searches XQuartz's /opt/X11 tree. Otherwise its Mesa libGL gets
# picked up as "OpenGL" instead of the system OpenGL.framework, and since SDL
# creates its GL context in the framework, mixing in a second libGL leaves
# every gl* call without a current context (glGetString returns null).

include(FetchContent)

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG v1.7.2)

FetchContent_Declare(cppembed
    GIT_REPOSITORY "https://github.com/jebreimo/cppembed.git"
    GIT_TAG "v0.1.10")

FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
    GIT_TAG v0.13.2)

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

find_package(OpenGL REQUIRED)
# On macOS the GLEW package must be built with the arm64-osx-tungsten overlay
# triplet (see vcpkg-triplets/ and CMakePresets.json); otherwise vcpkg bakes
# XQuartz's /opt/X11 libGL into GLEW's exported target, colliding with the
# OpenGL.framework SDL uses. Always configure via a CMake preset.
find_package(GLEW REQUIRED)
find_package(SDL3 REQUIRED)

add_library(Tungsten STATIC
    include/Tungsten/Detail/BuddyAllocator.hpp
    include/Tungsten/Detail/GenericBitmaskOperators.hpp
    include/Tungsten/DeviceInfo.hpp
    include/Tungsten/Environment.hpp
    include/Tungsten/Gl/DummyOglWrapper.hpp
    include/Tungsten/Gl/GlBuffer.hpp
    include/Tungsten/Gl/GlFramebuffer.hpp
    include/Tungsten/Gl/GlHandle.hpp
    include/Tungsten/Gl/GlProgram.hpp
    include/Tungsten/Gl/GlRendering.hpp
    include/Tungsten/Gl/GlSampler.hpp
    include/Tungsten/Gl/GlShader.hpp
    include/Tungsten/Gl/GlStateEpoch.hpp
    include/Tungsten/Gl/GlStateManagement.hpp
    include/Tungsten/Gl/GlTexture.hpp
    include/Tungsten/Gl/GlUniform.hpp
    include/Tungsten/Gl/GlVertexArray.hpp
    include/Tungsten/Gl/IOglWrapper.hpp
    include/Tungsten/Neo/BuiltinShaders.hpp
    include/Tungsten/Neo/CameraComponent.hpp
    include/Tungsten/Neo/ColorMaterials.hpp
    include/Tungsten/Neo/DoubleBuffer.hpp
    include/Tungsten/Neo/Font.hpp
    include/Tungsten/Neo/FontManager.hpp
    include/Tungsten/Neo/FontManager.hpp
    include/Tungsten/Neo/GlStateCache.hpp
    include/Tungsten/Neo/LightComponent.hpp
    include/Tungsten/Neo/LightData.hpp
    include/Tungsten/Neo/Material.hpp
    include/Tungsten/Neo/Mesh.hpp
    include/Tungsten/Neo/NodeHandle.hpp
    include/Tungsten/Neo/NodeId.hpp
    include/Tungsten/Neo/RenderItem.hpp
    include/Tungsten/Neo/RenderSnapshot.hpp
    include/Tungsten/Neo/RenderableComponent.hpp
    include/Tungsten/Neo/Renderer.hpp
    include/Tungsten/Neo/ResourceManager.hpp
    include/Tungsten/Neo/ResourceRef.hpp
    include/Tungsten/Neo/ResourceRefs.hpp
    include/Tungsten/Neo/Scene.hpp
    include/Tungsten/Neo/ShaderFamily.hpp
    include/Tungsten/Neo/ShaderPreprocessor.hpp
    include/Tungsten/Neo/ShaderProgram.hpp
    include/Tungsten/Neo/ShaderProgramBuilder.hpp
    include/Tungsten/Neo/ShaderVariantKey.hpp
    include/Tungsten/Neo/SharedBuffer.hpp
    include/Tungsten/Neo/SnapshotBuilder.hpp
    include/Tungsten/Neo/TextComponent.hpp
    include/Tungsten/Neo/TextStyle.hpp
    include/Tungsten/Neo/TextSystem.hpp
    include/Tungsten/Neo/Texture.hpp
    include/Tungsten/Neo/Transform.hpp
    include/Tungsten/Neo/VertexArray.hpp
    include/Tungsten/Neo/VertexAttribute.hpp
    include/Tungsten/Neo/VertexLayout.hpp
    include/Tungsten/Neo/VertexLayoutBuilder.hpp
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
    src/Tungsten/Detail/BuddyAllocator.cpp
    src/Tungsten/DeviceInfo.cpp
    src/Tungsten/ExceptionHelpers.cpp
    src/Tungsten/ExceptionHelpers.hpp
    src/Tungsten/Gl/GlBuffer.cpp
    src/Tungsten/Gl/GlFramebuffer.cpp
    src/Tungsten/Gl/GlProgram.cpp
    src/Tungsten/Gl/GlRendering.cpp
    src/Tungsten/Gl/GlSampler.cpp
    src/Tungsten/Gl/GlShader.cpp
    src/Tungsten/Gl/GlStateEpoch.cpp
    src/Tungsten/Gl/GlStateManagement.cpp
    src/Tungsten/Gl/GlTexture.cpp
    src/Tungsten/Gl/GlTypeConversion.cpp
    src/Tungsten/Gl/GlTypeConversion.hpp
    src/Tungsten/Gl/GlUniform.cpp
    src/Tungsten/Gl/GlVertexArray.cpp
    src/Tungsten/Gl/IOglWrapper.cpp
    src/Tungsten/Gl/OglWrapper.hpp
    src/Tungsten/Gl/TracingOglWrapper.hpp
    src/Tungsten/Neo/BufferArena.cpp
    src/Tungsten/Neo/BufferArena.hpp
    src/Tungsten/Neo/BuiltinShaders.cpp
    src/Tungsten/Neo/CameraComponent.cpp
    src/Tungsten/Neo/ColorMaterials.cpp
    src/Tungsten/Neo/DeletionQueue.cpp
    src/Tungsten/Neo/DeletionQueue.hpp
    src/Tungsten/Neo/Font.cpp
    src/Tungsten/Neo/FontManager.cpp
    src/Tungsten/Neo/FontUtilities.cpp
    src/Tungsten/Neo/FontUtilities.hpp
    src/Tungsten/Neo/Fonts/Monaco32.hpp
    src/Tungsten/Neo/Fonts/Monaco64.hpp
    src/Tungsten/Neo/GenerationalPool.hpp
    src/Tungsten/Neo/GlStateCache.cpp
    src/Tungsten/Neo/LayoutRegistry.cpp
    src/Tungsten/Neo/LayoutRegistry.hpp
    src/Tungsten/Neo/LightData.cpp
    src/Tungsten/Neo/RenderItem.cpp
    src/Tungsten/Neo/Renderer.cpp
    src/Tungsten/Neo/ResourceManager.cpp
    src/Tungsten/Neo/SamplerRegistry.cpp
    src/Tungsten/Neo/SamplerRegistry.hpp
    src/Tungsten/Neo/Scene.cpp
    src/Tungsten/Neo/ShaderLibrary.cpp
    src/Tungsten/Neo/ShaderLibrary.hpp
    src/Tungsten/Neo/ShaderPreprocessor.cpp
    src/Tungsten/Neo/ShaderProgramBuilder.cpp
    src/Tungsten/Neo/Shaders/BuiltinShaderSources.hpp
    src/Tungsten/Neo/SnapshotBuilder.cpp
    src/Tungsten/Neo/TextSystem.cpp
    src/Tungsten/Neo/TextUtilities.cpp
    src/Tungsten/Neo/TextUtilities.hpp
    src/Tungsten/Neo/UboBindings.hpp
    src/Tungsten/Neo/VaoCache.cpp
    src/Tungsten/Neo/VaoCache.hpp
    src/Tungsten/Neo/VertexArray.cpp
    src/Tungsten/Neo/VertexLayoutBuilder.cpp
    src/Tungsten/Sdl/CommandLine.cpp
    src/Tungsten/Sdl/CommandLine.hpp
    src/Tungsten/Sdl/SdlApplication.cpp
    src/Tungsten/Sdl/SdlDisplay.cpp
    src/Tungsten/Sdl/SdlGlContext.cpp
    src/Tungsten/Sdl/SdlGlParameters.cpp
    src/Tungsten/Sdl/SdlSession.cpp
    src/Tungsten/Viewport.cpp
    src/Tungsten/YimageGl.cpp
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
    src/Tungsten/Neo/Shaders/BuiltinShaderSources.cpp.in
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
