//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderManager.hpp"

#include "Tungsten/GouraudShader.hpp"
#include "Tungsten/PhongShader.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
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

    ShaderProgram& ShaderManager::get_program(const std::string& name)
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

    ShaderProgram& ShaderManager::get_program(BuiltinShaders program)
    {
        switch (program)
        {
        case BuiltinShaders::PHONG:
            return get_or_create_builtin_program<PhongShader>();
        case BuiltinShaders::GOURAUD:
            return get_or_create_builtin_program<GouraudShader>();
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
        return get_program(name);
    }
}
