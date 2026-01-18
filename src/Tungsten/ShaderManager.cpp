//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderManager.hpp"

#include "Tungsten/ShaderPrograms/BlinnPhongShader.hpp"
#include "Tungsten/ShaderPrograms/GouraudShader.hpp"
#include "Tungsten/ShaderPrograms/PhongShader.hpp"
#include "Tungsten/ShaderPrograms/TexturedBlinnPhongShader.hpp"
#include "Tungsten/ShaderPrograms/TexturedGouraudShader.hpp"
#include "Tungsten/ShaderPrograms/TexturedPhongShader.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    BuiltinShader to_builtin_shader(const std::string& name)
    {
        if (name == "GOURAUD" || name == GouraudShader::NAME)
            return BuiltinShader::GOURAUD;
        if (name == "PHONG" || name == PhongShader::NAME)
            return BuiltinShader::PHONG;
        if (name == "BLINN_PHONG" || name == BlinnPhongShader::NAME)
            return BuiltinShader::BLINN_PHONG;
        if (name == "TEXTURED_GOURAUD" || name == TexturedGouraudShader::NAME)
            return BuiltinShader::TEXTURED_GOURAUD;
        if (name == "TEXTURED_PHONG" || name == TexturedPhongShader::NAME)
            return BuiltinShader::TEXTURED_PHONG;
        if (name == "TEXTURED_BLINN_PHONG" || name == TexturedBlinnPhongShader::NAME)
            return BuiltinShader::TEXTURED_BLINN_PHONG;
        TUNGSTEN_THROW("Unknown built-in shader program name: '" + name + "'.");
    }

    ShaderManager& ShaderManager::instance()
    {
        static ShaderManager instance;
        return instance;
    }

    void ShaderManager::add_program(std::unique_ptr<ShaderProgram> program)
    {
        if (!programs_.insert({program->name(), std::move(program)}).second)
            TUNGSTEN_THROW("Program with name '" + program->name() + "' already exists.");
    }

    ShaderProgram& ShaderManager::program(const std::string& name)
    {
        const auto program = try_get_program(name);
        if (!program)
            TUNGSTEN_THROW("No program with name '" + name + "' found.");
        return *program;
    }

    ShaderProgram* ShaderManager::try_get_program(const std::string& name)
    {
        const auto it = programs_.find(name);
        if (it == programs_.end())
            return nullptr;
        return it->second.get();
    }

    ShaderProgram& ShaderManager::program(BuiltinShader program)
    {
        switch (program)
        {
        case BuiltinShader::GOURAUD:
            return get_or_create_builtin_program<GouraudShader>();
        case BuiltinShader::PHONG:
            return get_or_create_builtin_program<PhongShader>();
        case BuiltinShader::BLINN_PHONG:
            return get_or_create_builtin_program<BlinnPhongShader>();
        case BuiltinShader::TEXTURED_GOURAUD:
            return get_or_create_builtin_program<TexturedGouraudShader>();
        case BuiltinShader::TEXTURED_PHONG:
            return get_or_create_builtin_program<TexturedPhongShader>();
        case BuiltinShader::TEXTURED_BLINN_PHONG:
            return get_or_create_builtin_program<TexturedBlinnPhongShader>();
        default:
            TUNGSTEN_THROW("Unknown built-in shader program.");
        }
    }

    template <typename ShaderType>
    ShaderProgram& ShaderManager::get_or_create_builtin_program()
    {
        const std::string name(ShaderType::NAME);
        if (const auto ptr = try_get_program(name))
            return *ptr;

        auto phong_shader = std::make_unique<ShaderType>();
        add_program(std::move(phong_shader));
        return program(name);
    }

    ShaderPreprocessor& ShaderManager::preprocessor()
    {
        return preprocessor_;
    }
}
