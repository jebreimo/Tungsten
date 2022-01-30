//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 23.04.2017.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderTools.hpp"

#include <fstream>

namespace Tungsten
{
    namespace
    {
        std::string readFile(const std::string& path)
        {
            std::ifstream file(path);
            if (!file)
                throw std::runtime_error(path + ": Unable to open file.");

            std::string result;
            file.seekg(0, std::ios::end);
            auto fileSize = static_cast<size_t>(file.tellg());
            result.resize(fileSize);
            if (result.size() < fileSize)
            {
                throw std::runtime_error(path + ": File is too big. Size = "
                                         + std::to_string(fileSize));
            }
            file.seekg(0, std::ios::beg);
            file.read(&result[0], fileSize);
            return result;
        }
    }

    std::string read_shader(const std::string& path)
    {
        return readFile(path);
    }

    Tungsten::ShaderHandle create_shader(GLuint shader_type,
                                         const std::string& source_code)
    {
        auto shader = Tungsten::create_shader(shader_type);
        Tungsten::set_shader_source(shader, source_code);
        Tungsten::compile_shader(shader);
        return shader;
    }

    Tungsten::ShaderHandle read_and_compile_shader(GLuint shader_type,
                                                   const std::string& path)
    {
        auto source = read_shader(path);
        return create_shader(shader_type, source);
    }
}
