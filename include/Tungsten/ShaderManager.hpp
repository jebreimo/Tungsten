//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <unordered_map>

#include "ShaderProgram.hpp"
#include "ShaderPreprocessor.hpp"

namespace Tungsten
{
    enum class BuiltinShaders
    {
        GOURAUD,
        PHONG,
        TEXTURED_PHONG
    };

    class ShaderManager
    {
    public:
        [[nodiscard]] static ShaderManager& instance();

        void add_program(std::unique_ptr<ShaderProgram> program);

        [[nodiscard]] ShaderProgram& program(const std::string& name);

        [[nodiscard]] ShaderProgram* try_get_program(const std::string& name);

        [[nodiscard]] ShaderProgram& program(BuiltinShaders program);

        [[nodiscard]] ShaderPreprocessor& preprocessor();
    private:
        ShaderManager() = default;

        template <typename ShaderType>
        ShaderProgram& get_or_create_builtin_program();

        std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> programs_;
        ShaderPreprocessor preprocessor_;
    };
}
