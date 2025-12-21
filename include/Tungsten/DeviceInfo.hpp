//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iosfwd>
#include <string>
#include <vector>

namespace Tungsten
{
    struct DeviceInfo
    {
        std::string vendor;
        std::string renderer;
        std::string version;
        std::string shader_language_version;

        int32_t max_vertex_attribs;
        int32_t max_vertex_uniform_vectors;
        int32_t max_vertex_output_components;
        int32_t max_fragment_input_components;
        int32_t max_vertex_texture_image_units;
        int32_t max_combined_texture_image_units;
        int32_t max_texture_image_units;
        int32_t max_fragment_uniform_vectors;
        int32_t max_draw_buffers;
        int32_t min_program_texel_offset;
        int32_t max_program_texel_offset;

        std::vector<std::string> extensions;
    };

    std::ostream& operator<<(std::ostream& os, const DeviceInfo& info);

    [[nodiscard]] DeviceInfo get_device_info();

    [[nodiscard]] std::string get_gl_version();

    [[nodiscard]] std::string get_shader_language_version();

    [[nodiscard]] std::string get_shader_language_version_string();
}
