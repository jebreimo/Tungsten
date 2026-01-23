//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <unordered_map>

#include "../Gl/GlProgram.hpp"
#include "../ShaderPreprocessor.hpp"
#include "../VertexArrayObject.hpp"

namespace Tungsten
{
    enum class AttributeKind
    {
        POSITION,
        NORMAL,
        TEXCOORD,
        COLOR,
        TANGENT,
        BITANGENT,
        CUSTOM
    };

    class Attribute
    {
    public:
        std::string name;
        int32_t size = 0;
        uint32_t location = 0;
        AttributeKind kind = AttributeKind::CUSTOM;
        AttributeDataType value_type = AttributeDataType::FLOAT;
    };

    class Uniform2
    {
    public:
        std::string name;
        uint32_t location = 0;
        UniformType value_type = UniformType::FLOAT;
    };
    class ShaderProgram
    {
    public:
        virtual ~ShaderProgram();

        [[nodiscard]] const std::string& name() const;

        [[nodiscard]] const std::string& source(ShaderType type) const;

        void use() const;

        [[nodiscard]] const ProgramHandle& program() const;

        [[nodiscard]] virtual VertexArrayObject create_vao() const;

        [[nodiscard]] virtual VertexArrayObject create_vao(int32_t extra_stride) const = 0;

    protected:
        ShaderProgram(std::string name,
                      std::vector<std::pair<ShaderType, std::string>> sources);

        ShaderProgram(std::string name,
                      std::vector<std::pair<ShaderType, std::string>> sources,
                      const ShaderPreprocessor& preprocessor);

    private:
        std::string name_;
        std::unordered_map<ShaderType, std::string> sources_;
        ProgramHandle program_;
    };
}
