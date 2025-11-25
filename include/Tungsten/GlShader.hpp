//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct ShaderDeleter
    {
        void operator()(uint32_t id) const;
    };

    using ShaderHandle = GlHandle<ShaderDeleter>;

    void compile_shader(uint32_t shader_id);

    ShaderHandle create_shader(uint32_t shader_type);

    void set_shader_source(uint32_t shader_id, const std::string& source);

    bool get_shader_compile_status(uint32_t shader_id);

    std::string get_shader_info_log(uint32_t shader_id);

    int32_t get_shader_info_log_length(uint32_t shader_id);
}
