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
    struct GlShaderDeleter
    {
        void operator()(GLuint id) const;
    };

    using ShaderHandle = GlHandle<GlShaderDeleter>;

    void compile_shader(GLuint shader_id);

    ShaderHandle create_shader(GLuint shaderType);

    void set_shader_source(GLuint shader_id, const std::string& source);

    bool get_shader_compile_status(GLuint shader_id);

    std::string get_shader_info_log(GLuint shader_id);

    GLint get_shader_info_log_length(GLuint shader_id);
}
