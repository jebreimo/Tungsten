##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-12-24.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.13)

set_property(GLOBAL
    PROPERTY
        tungsten_cmake_module_dir "${CMAKE_CURRENT_LIST_DIR}")

function(tungsten_target_embed_shaders targetName)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    # TODO: Add FILE_SUFFIX
    cmake_parse_arguments(ARG "" "" "FILES" ${ARGN})

    foreach (SHADER_PATH IN LISTS ARG_FILES)
        set(SHADER_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_PATH}")
        get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)
        set(SHADER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tungsten_include/${SHADER_NAME}.hpp")
        get_property(SCRIPT_DIR GLOBAL PROPERTY tungsten_cmake_module_dir)
        set(SCRIPT_PATH ${SCRIPT_DIR}/make_cpp_string.py)
        add_custom_command(OUTPUT "${SHADER_OUTPUT}"
            COMMAND "${Python3_EXECUTABLE}" ${SCRIPT_PATH} "${SHADER_INPUT}" "${SHADER_OUTPUT}"
            DEPENDS "${SHADER_INPUT}")
        list(APPEND SHADER_OUTPUT_FILES "${SHADER_OUTPUT}")
    endforeach ()

    add_custom_target(${targetName}_ShaderHeaders ALL
        DEPENDS ${SHADER_OUTPUT_FILES})

    add_dependencies(${targetName} ${targetName}_ShaderHeaders)

    target_include_directories(${targetName} BEFORE
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/tungsten_include>
        )
endfunction()
