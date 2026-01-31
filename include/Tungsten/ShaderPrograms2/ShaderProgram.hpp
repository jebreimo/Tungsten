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
    enum class AttributeType
    {
        POSITION,
        NORMAL,
        TEX_COORD,
        COLOR,
        TANGENT,
        BITANGENT,
        CUSTOM
    };

    enum class UniformType
    {
        INT,
        FLOAT,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        MATRIX2,
        MATRIX3,
        MATRIX4
    };

    class Attribute
    {
    public:
        std::string name;
        int32_t size = 0;
        uint32_t location = 0;
        AttributeType type = AttributeType::CUSTOM;
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

        [[nodiscard]] const ProgramHandle& handle() const;

        [[nodiscard]] virtual VertexArrayObject create_vao() const;

        [[nodiscard]] virtual VertexArrayObject create_vao(int32_t extra_stride) const = 0;

        [[nodiscard]] const std::vector<Attribute>& attributes() const;

        [[nodiscard]] const std::vector<Uniform2>& uniforms() const;

        [[nodiscard]] const Uniform2* get_uniform(const std::string& name) const;

        void set_uniform(const std::string& name, int32_t value);
        void set_uniform(const std::string& name, float value);
        void set_uniform(const std::string& name, const Xyz::Vector2F& value);
        void set_uniform(const std::string& name, const Xyz::Vector3F& value);
        void set_uniform(const std::string& name, const Xyz::Vector4F& value);
        void set_uniform(const std::string& name, const Xyz::Matrix2F& value);
        void set_uniform(const std::string& name, const Xyz::Matrix3F& value);
        void set_uniform(const std::string& name, const Xyz::Matrix4F& value);
    protected:
        ShaderProgram(std::string name,
                      std::vector<std::pair<ShaderType, std::string>> sources);

        ShaderProgram(std::string name,
                      std::vector<std::pair<ShaderType, std::string>> sources,
                      const ShaderPreprocessor& preprocessor);

    private:
        std::string name_;
        std::unordered_map<ShaderType, std::string> sources_;
        std::unordered_map<std::string, Uniform2> uniforms_;
        ProgramHandle program_;
    };
}
