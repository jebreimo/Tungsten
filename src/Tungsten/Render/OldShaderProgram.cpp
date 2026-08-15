//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/OldShaderProgram.hpp"

#include "Tungsten/Render/ShaderManager.hpp"
#include "Tungsten/Render/ShaderProgramBuilder.hpp"

namespace Tungsten
{
    OldShaderProgram::OldShaderProgram(std::string name,
                                 std::vector<std::pair<ShaderType, std::string>> sources)
        : OldShaderProgram(std::move(name),
                        std::move(sources),
                        ShaderManager::instance().preprocessor())
    {
    }

    OldShaderProgram::OldShaderProgram(std::string name,
                                 std::vector<std::pair<ShaderType, std::string>> sources,
                                 const ShaderPreprocessor& preprocessor)
        : name_(std::move(name))
    {
        ShaderProgramBuilder builder;
        for (auto& [type, source] : sources)
        {
            builder.add_shader(type, preprocessor.preprocess(source, {}));
            sources_.insert({type, std::move(source)});
        }
        program_ = builder.build();
    }

    OldShaderProgram::~OldShaderProgram() = default;

    const std::string& OldShaderProgram::name() const
    {
        return name_;
    }

    const std::string& OldShaderProgram::source(ShaderType type) const
    {
        return sources_.at(type);
    }

    void OldShaderProgram::use() const
    {
        use_program(program_.id());
    }

    const ProgramHandle& OldShaderProgram::handle() const
    {
        return program_;
    }

    uint32_t OldShaderProgram::id() const
    {
        return program_.id();
    }

    const std::vector<VertexAttributeDefinition>&
    OldShaderProgram::attribute_definitions() const
    {
        return attribute_definitions_;
    }

    void OldShaderProgram::set_attribute_definitions(
        std::vector<VertexAttributeDefinition> definitions)
    {
        attribute_definitions_ = std::move(definitions);
    }
}
