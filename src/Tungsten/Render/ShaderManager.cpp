//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/ShaderManager.hpp"

#include "Tungsten/Render/OldShaderProgram.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    ShaderManager& ShaderManager::instance()
    {
        static ShaderManager instance;
        return instance;
    }

    void ShaderManager::add_program(std::unique_ptr<OldShaderProgram> program)
    {
        if (!programs_.insert({program->name(), std::move(program)}).second)
            TUNGSTEN_THROW("Program with name '" + program->name() + "' already exists.");
    }

    OldShaderProgram& ShaderManager::program(const std::string& name)
    {
        const auto program = try_get_program(name);
        if (!program)
            TUNGSTEN_THROW("No program with name '" + name + "' found.");
        return *program;
    }

    OldShaderProgram* ShaderManager::try_get_program(const std::string& name)
    {
        const auto it = programs_.find(name);
        if (it == programs_.end())
            return nullptr;
        return it->second.get();
    }

    ShaderPreprocessor& ShaderManager::preprocessor()
    {
        return preprocessor_;
    }
}
