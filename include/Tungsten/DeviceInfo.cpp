//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "DeviceInfo.hpp"

#include <GL/glew.h>

#include "IOglWrapper.hpp"

namespace Tungsten
{
    DeviceInfo get_device_info()
    {
        auto& ogl = get_ogl_wrapper();
        DeviceInfo device;
        {
            const char* str = reinterpret_cast<const char*>(ogl.get_string(GL_VENDOR));
            device.vendor = str ? str : "";
        }
        {
            const char* str = reinterpret_cast<const char*>(ogl.get_string(GL_RENDERER));
            device.renderer = str ? str : "";
        }
        {
            const char* str = reinterpret_cast<const char*>(ogl.get_string(GL_VERSION));
            device.version = str ? str : "";
        }
        {
            const char* str = reinterpret_cast<const char*>(
                ogl.get_string(GL_SHADING_LANGUAGE_VERSION));
            device.shader_language_version = str ? str : "";
        }
        {
            const char* str = reinterpret_cast<const char*>(ogl.get_string(GL_EXTENSIONS));
            if (str)
            {
                std::string extensions_str = str;
                size_t start = 0;
                size_t end;
                while ((end = extensions_str.find(' ', start)) != std::string::npos)
                {
                    device.extensions.push_back(
                        extensions_str.substr(start, end - start));
                    start = end + 1;
                }
                if (start < extensions_str.size())
                {
                    device.extensions.push_back(
                        extensions_str.substr(start));
                }
            }
        }
        return device;
    }
}
