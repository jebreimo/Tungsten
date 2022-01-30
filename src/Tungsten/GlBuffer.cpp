//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlBuffer.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlBufferDeleter::operator()(GLuint id) const
    {
        glDeleteBuffers(1, &id);
        THROW_IF_GL_ERROR();
    }

    void bind_buffer(GLenum target, GLuint buffer)
    {
        glBindBuffer(target, buffer);
        THROW_IF_GL_ERROR();
    }

    BufferHandle generate_buffer()
    {
        GLuint id;
        glGenBuffers(1, &id);
        THROW_IF_GL_ERROR();
        return BufferHandle(id);
    }

    std::vector<BufferHandle> generate_buffers(GLsizei count)
    {
        if (count == 0)
            return std::vector<BufferHandle>();

        auto ids = std::vector<GLuint>(size_t(count));
        glGenBuffers(count, ids.data());
        THROW_IF_GL_ERROR();
        auto result = std::vector<BufferHandle>();
        for (auto id : ids)
            result.emplace_back(id);
        return result;
    }

    void set_buffer_data(GLenum target, GLsizeiptr size, const GLvoid* data,
                         GLenum usage)
    {
        glBufferData(target, size, data, usage);
        THROW_IF_GL_ERROR();
    }

    void set_element_array_buffer(GLuint buffer_id,
                                  GLsizeiptr value_count,
                                  const uint16_t* values,
                                  GLenum usage)
    {
        bind_buffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
        set_buffer_data(GL_ELEMENT_ARRAY_BUFFER,
                        GLsizeiptr(value_count * sizeof(uint16_t)),
                        values,
                        usage);
    }
}
