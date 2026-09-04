//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/ShaderProgramBuilder.hpp"

#include <filesystem>
#include <fstream>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        std::string read_shader(const std::filesystem::path& path)
        {
            std::ifstream file(path);
            if (!file)
                TUNGSTEN_THROW("" + path.string() + ": Unable to open file.");

            std::string result;
            file.seekg(0, std::ios::end);
            auto file_size = static_cast<size_t>(file.tellg());
            result.resize(file_size);
            if (result.size() < file_size)
            {
                TUNGSTEN_THROW("" + path.string() + ": File is too big. Size = "
                    + std::to_string(file_size));
            }
            file.seekg(0, std::ios::beg);
            file.read(&result[0], static_cast<std::streamsize>(file_size));
            return result;
        }

        ShaderHandle create_shader(ShaderType shader_type,
                                   const std::string& source_code)
        {
            auto shader = create_shader(shader_type);
            set_shader_source(shader.id(), source_code);
            compile_shader(shader.id());
            return shader;
        }
    }

    ShaderProgramBuilder::ShaderProgramBuilder()
        : program_(create_program())
    {
    }

    ShaderProgramBuilder&
    ShaderProgramBuilder::add_shader(ShaderType type, const std::string& src)
    {
        auto shader = create_shader(type, src);
        attach_shader(program_.id(), shader.id());
        return *this;
    }

    ShaderProgramBuilder&
    ShaderProgramBuilder::add_shader(ShaderType type, const std::filesystem::path& path)
    {
        const auto source = read_shader(path);
        return add_shader(type, source);
    }

    ProgramHandle ShaderProgramBuilder::build()
    {
        link_program(program_.id());
        return std::move(program_);
    }
}
