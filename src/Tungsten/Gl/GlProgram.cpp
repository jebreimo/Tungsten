//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlProgram.hpp"

#include <GL/glew.h>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void ProgramDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().delete_program(id);
        THROW_IF_GL_ERROR();
    }

    ProgramHandle create_program()
    {
        auto id = get_ogl_wrapper().create_program();
        THROW_IF_GL_ERROR();
        return ProgramHandle(id);
    }

    void attach_shader(uint32_t program_id, uint32_t shader_id)
    {
        get_ogl_wrapper().attach_shader(program_id, shader_id);
        THROW_IF_GL_ERROR();
    }

    uint32_t get_vertex_attribute(uint32_t program_id, const std::string& name)
    {
        auto attrLoc = get_ogl_wrapper().get_attrib_location(program_id, name.c_str());
        THROW_IF_GL_ERROR();
        return uint32_t(attrLoc);
    }

    int32_t get_uniform_location(uint32_t program_id, const char* name)
    {
        auto result = get_ogl_wrapper().get_uniform_location(program_id, name);
        THROW_IF_GL_ERROR();
        return result;
    }

    bool get_program_link_status(uint32_t program_id)
    {
        int32_t result = 0;
        get_ogl_wrapper().get_program(program_id, GL_LINK_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    int32_t get_program_info_log_length(uint32_t program_id)
    {
        int32_t size;
        get_ogl_wrapper().get_program(program_id, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string get_program_info_log(uint32_t program_id)
    {
        auto size = get_program_info_log_length(program_id);
        std::string result(size, '\0');
        get_ogl_wrapper().get_program_info_log(program_id, size, &size, result.data());
        return result;
    }

    void link_program(uint32_t program_id)
    {
        get_ogl_wrapper().link_program(program_id);
        THROW_IF_GL_ERROR();
        if (!get_program_link_status(program_id))
            TUNGSTEN_THROW(+ get_program_info_log(program_id));
    }

    void use_program(uint32_t program_id)
    {
        get_ogl_wrapper().use_program(program_id);
        THROW_IF_GL_ERROR();
    }
}
