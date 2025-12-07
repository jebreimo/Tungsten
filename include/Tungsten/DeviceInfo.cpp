//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "DeviceInfo.hpp"

#include <ostream>
#include <GL/glew.h>

#include "IOglWrapper.hpp"
#include "TungstenException.hpp"

namespace Tungsten
{
    std::ostream& operator<<(std::ostream& os, const DeviceInfo& info)
    {
        os << "Vendor: " << info.vendor << '\n'
            << "Renderer: " << info.renderer << '\n'
            << "Version: " << info.version << '\n'
            << "Shader Language Version: "
            << info.shader_language_version << '\n';
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
            .extensions = get_extension_list()
        };
    }
}
