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
    include/Tungsten/Rendering/DoubleBuffer.hpp
    include/Tungsten/Rendering/Font.hpp
    include/Tungsten/Rendering/FontManager.hpp
    include/Tungsten/Rendering/RenderItem.hpp
    include/Tungsten/Rendering/RenderSnapshot.hpp
    include/Tungsten/Rendering/Renderer.hpp
    include/Tungsten/Rendering/SnapshotBuilder.hpp
    include/Tungsten/Rendering/TextStyle.hpp
    include/Tungsten/Rendering/TextSystem.hpp
    include/Tungsten/Resources/BuiltinShaders.hpp
    include/Tungsten/Resources/ColorMaterials.hpp
    include/Tungsten/Resources/GlStateCache.hpp
    include/Tungsten/Resources/LightData.hpp
    include/Tungsten/Resources/Material.hpp
    include/Tungsten/Resources/Mesh.hpp
    include/Tungsten/Resources/ResourceManager.hpp
    include/Tungsten/Resources/ResourceRef.hpp
    include/Tungsten/Resources/ResourceRefs.hpp
    include/Tungsten/Resources/ShaderFamily.hpp
    include/Tungsten/Resources/ShaderPreprocessor.hpp
    include/Tungsten/Resources/ShaderProgram.hpp
    include/Tungsten/Resources/ShaderProgramBuilder.hpp
    include/Tungsten/Resources/ShaderVariantKey.hpp
    include/Tungsten/Resources/SharedBuffer.hpp
    include/Tungsten/Resources/Texture.hpp
    include/Tungsten/Resources/VertexArray.hpp
    include/Tungsten/Resources/VertexAttribute.hpp
    include/Tungsten/Resources/VertexLayout.hpp
    include/Tungsten/Resources/VertexLayoutBuilder.hpp
    include/Tungsten/SceneGraph/CameraComponent.hpp
    include/Tungsten/SceneGraph/LightComponent.hpp
    include/Tungsten/SceneGraph/NodeHandle.hpp
    include/Tungsten/SceneGraph/NodeId.hpp
    include/Tungsten/SceneGraph/RenderableComponent.hpp
    include/Tungsten/SceneGraph/Scene.hpp
    include/Tungsten/SceneGraph/TextComponent.hpp
    include/Tungsten/SceneGraph/Transform.hpp
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
    src/Tungsten/Rendering/Font.cpp
    src/Tungsten/Rendering/FontManager.cpp
    src/Tungsten/Rendering/FontUtilities.cpp
    src/Tungsten/Rendering/FontUtilities.hpp
    src/Tungsten/Rendering/Fonts/Monaco32.hpp
    src/Tungsten/Rendering/Fonts/Monaco64.hpp
    src/Tungsten/Rendering/RenderItem.cpp
    src/Tungsten/Rendering/Renderer.cpp
    src/Tungsten/Rendering/SnapshotBuilder.cpp
    src/Tungsten/Rendering/TextSystem.cpp
    src/Tungsten/Rendering/TextUtilities.cpp
    src/Tungsten/Rendering/TextUtilities.hpp
    src/Tungsten/Resources/BufferArena.cpp
    src/Tungsten/Resources/BufferArena.hpp
    src/Tungsten/Resources/BuiltinShaders.cpp
    src/Tungsten/Resources/ColorMaterials.cpp
    src/Tungsten/Resources/DeletionQueue.cpp
    src/Tungsten/Resources/DeletionQueue.hpp
    src/Tungsten/Resources/GenerationalPool.hpp
    src/Tungsten/Resources/GlStateCache.cpp
    src/Tungsten/Resources/LayoutRegistry.cpp
    src/Tungsten/Resources/LayoutRegistry.hpp
    src/Tungsten/Resources/LightData.cpp
    src/Tungsten/Resources/ResourceManager.cpp
    src/Tungsten/Resources/SamplerRegistry.cpp
    src/Tungsten/Resources/SamplerRegistry.hpp
    src/Tungsten/Resources/ShaderLibrary.cpp
    src/Tungsten/Resources/ShaderLibrary.hpp
    src/Tungsten/Resources/ShaderPreprocessor.cpp
    src/Tungsten/Resources/ShaderProgramBuilder.cpp
    src/Tungsten/Resources/Shaders/BuiltinShaderSources.hpp
    src/Tungsten/Resources/UboBindings.hpp
    src/Tungsten/Resources/VaoCache.cpp
    src/Tungsten/Resources/VaoCache.hpp
    src/Tungsten/Resources/VertexArray.cpp
    src/Tungsten/Resources/VertexLayoutBuilder.cpp
    src/Tungsten/SceneGraph/CameraComponent.cpp
    src/Tungsten/SceneGraph/Scene.cpp
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
    src/Tungsten/Resources/Shaders/BuiltinShaderSources.cpp.in
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
