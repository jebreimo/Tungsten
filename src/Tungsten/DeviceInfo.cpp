//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../include/Tungsten/DeviceInfo.hpp"

#include <ostream>
#include <GL/glew.h>

#include "../../include/Tungsten/IOglWrapper.hpp"
#include "../../include/Tungsten/TungstenException.hpp"

namespace Tungsten
{
    std::ostream& operator<<(std::ostream& os, const DeviceInfo& info)
    {
        os << "Vendor: " << info.vendor << '\n'
            << "Renderer: " << info.renderer << '\n'
            << "Version: " << info.version << '\n'
            << "Shader Language Version: "
            << info.shader_language_version << '\n'
        << "Built-in constants:\n"
            << "  Max Vertex Attribs: " << info.max_vertex_attribs << '\n'
            << "  Max Vertex Uniform Vectors: "
            << info.max_vertex_uniform_vectors << '\n'
            << "  Max Vertex Output Components: "
            << info.max_vertex_output_components << '\n'
            << "  Max Fragment Input Components: "
            << info.max_fragment_input_components << '\n'
            << "  Max Vertex Texture Image Units: "
            << info.max_vertex_texture_image_units << '\n'
            << "  Max Combined Texture Image Units: "
            << info.max_combined_texture_image_units << '\n'
            << "  Max Texture Image Units: "
            << info.max_texture_image_units << '\n'
            << "  Max Fragment Uniform Vectors: "
            << info.max_fragment_uniform_vectors << '\n'
            << "  Max Draw Buffers: "
            << info.max_draw_buffers << '\n'
            << "  Min Program Texel Offset: "
            << info.min_program_texel_offset << '\n'
            << "  Max Program Texel Offset: "
            << info.max_program_texel_offset << '\n';
        os << "Extensions:";
        for (const auto& ext : info.extensions)
            os << "\n  " << ext;
        return os;
    }

    namespace
    {
        std::string get_string_or_empty(GLenum name)
        {
            auto& ogl = get_ogl_wrapper();
            const char* str = reinterpret_cast<const char*>(ogl.get_string(name));
            THROW_IF_GL_ERROR();
            return str ? str : "";
        }

        int32_t get_integer(GLenum name)
        {
            auto& ogl = get_ogl_wrapper();
            int32_t value;
            ogl.get_integer(name, &value);
            THROW_IF_GL_ERROR();
            return value;
        }

        std::vector<std::string> get_extension_list()
        {
            auto& ogl = get_ogl_wrapper();
            std::vector<std::string> extensions;
            uint32_t i = 0;
            while (true)
            {
                const char* str = reinterpret_cast<const char*>(ogl.get_string_i(GL_EXTENSIONS, i));
                if (str == nullptr)
                {
                    auto error = ogl.get_error();
                    if (error == GL_INVALID_VALUE)
                        break;
                    THROW_GL_ERROR(error);
                }
                extensions.emplace_back(reinterpret_cast<const char*>(str));
                ++i;
            }

            return extensions;
        }
    }

    DeviceInfo get_device_info()
    {
        auto& ogl = get_ogl_wrapper();
        return {
            .vendor = get_string_or_empty(GL_VENDOR),
            .renderer = get_string_or_empty(GL_RENDERER),
            .version = get_string_or_empty(GL_VERSION),
            .shader_language_version = get_string_or_empty(GL_SHADING_LANGUAGE_VERSION),
            .max_vertex_attribs = get_integer(GL_MAX_VERTEX_ATTRIBS),
            .max_vertex_uniform_vectors = get_integer(GL_MAX_VERTEX_UNIFORM_VECTORS),
            .max_vertex_output_components = get_integer(GL_MAX_VERTEX_OUTPUT_COMPONENTS),
            .max_fragment_input_components = get_integer(GL_MAX_FRAGMENT_INPUT_COMPONENTS),
            .max_vertex_texture_image_units = get_integer(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS),
            .max_combined_texture_image_units = get_integer(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS),
            .max_texture_image_units = get_integer(GL_MAX_TEXTURE_IMAGE_UNITS),
            .max_fragment_uniform_vectors = get_integer(GL_MAX_FRAGMENT_UNIFORM_VECTORS),
            .max_draw_buffers = get_integer(GL_MAX_DRAW_BUFFERS),
            .min_program_texel_offset = get_integer(GL_MIN_PROGRAM_TEXEL_OFFSET),
            .max_program_texel_offset = get_integer(GL_MAX_PROGRAM_TEXEL_OFFSET),
            .extensions = get_extension_list()
        };
    }
}
