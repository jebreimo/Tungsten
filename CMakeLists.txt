##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-04-22.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.15)

project(Tungsten VERSION 0.2.69)

set(CMAKE_CXX_STANDARD 17)

# Set TUNGSTEN_MASTER_PROJECT to ON if Xyz is the root level project.
if (NOT DEFINED TUNGSTEN_MASTER_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TUNGSTEN_MASTER_PROJECT ON)
    else ()
        set(TUNGSTEN_MASTER_PROJECT OFF)
    endif ()
endif ()

# # Test option
# option(TUNGSTEN_BUILD_TEST "Build tests" ${TUNGSTEN_MASTER_PROJECT})
#
# # Install option
# option(TUNGSTEN_INSTALL "Generate the install target" ${TUNGSTEN_MASTER_PROJECT})

if (EMSCRIPTEN)
    list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten)
else ()
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
endif ()

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR OR DEFINED ENV{LOCAL_SOURCE_PACKAGE_DIR})
    if (NOT LOCAL_SOURCE_PACKAGE_DIR)
        set(LOCAL_SOURCE_PACKAGE_DIR $ENV{LOCAL_SOURCE_PACKAGE_DIR})
    endif ()
endif ()

include(FetchContent)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR)
    FetchContent_Declare(xyz
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Xyz-0.2.75.76.tar.gz")
else ()
    FetchContent_Declare(xyz
        GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
        GIT_TAG v0.2.75)
endif ()

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG "v1.1.3")

FetchContent_MakeAvailable(argos xyz)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_library(Tungsten STATIC
    include/Tungsten/ArrayBuffer.hpp
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
    src/Tungsten/ShaderTools.cpp
    src/Tungsten/TungstenException.cpp
    src/Tungsten/Uniform.cpp
    )

target_link_libraries(Tungsten
    PUBLIC
        GLEW::GLEW
        OpenGL::GL
        SDL2::SDL2
        Xyz::Xyz
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

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR OR DEFINED ENV{LOCAL_SOURCE_PACKAGE_DIR})
    if (NOT LOCAL_SOURCE_PACKAGE_DIR)
        set(LOCAL_SOURCE_PACKAGE_DIR $ENV{LOCAL_SOURCE_PACKAGE_DIR})
    endif ()
    if (TUNGSTEN_MASTER_PROJECT)
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE CURRENT_GIT_REVISION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        add_custom_target(${PROJECT_NAME}_BUILD_SRC_TARBALL
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${GIT_EXECUTABLE} archive -v --format=tar.gz --prefix=${PROJECT_NAME}/ HEAD > ${LOCAL_SOURCE_PACKAGE_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.${CURRENT_GIT_REVISION}.tar.gz
            COMMAND echo Created ${LOCAL_SOURCE_PACKAGE_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.${CURRENT_GIT_REVISION}.tar.gz
            )
    endif ()
endif ()
