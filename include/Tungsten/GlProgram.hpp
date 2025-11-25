//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlShader.hpp"
#include "Uniform.hpp"

namespace Tungsten
{
    struct ProgramDeleter
    {
        void operator()(uint32_t id) const;
    };

    using ProgramHandle = GlHandle<ProgramDeleter>;

    ProgramHandle create_program();

    void attach_shader(uint32_t program_id, uint32_t shader_id);

    uint32_t get_vertex_attribute(uint32_t program_id, const std::string& name);

    int32_t get_uniform_location(uint32_t program_id, const char* name);

    template <typename T>
    Uniform<T> get_uniform(uint32_t program_id, const char* name)
    {
        return Uniform<T>(get_uniform_location(program_id, name));
    }

    void link_program(uint32_t program_id);

    void use_program(uint32_t program_id);
}
