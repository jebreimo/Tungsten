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
    void BufferDeleter::operator()(GLuint id) const
    {
        glDeleteBuffers(1, &id);
        THROW_IF_GL_ERROR();
    }

    BufferHandle generate_buffer()
    {
        GLuint id;
        glGenBuffers(1, &id);
        THROW_IF_GL_ERROR();
        return BufferHandle(id);
    }

    void generate_buffers(std::span<BufferHandle> buffers)
    {
        auto ids = std::vector<GLuint>(size_t(buffers.size()));
        glGenBuffers(GLsizei(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            buffers[i] = BufferHandle(ids[i]);
    }

    void bind_buffer(GLenum target, GLuint buffer)
    {
        glBindBuffer(target, buffer);
        THROW_IF_GL_ERROR();
    }

    void set_buffer_data(GLenum target, GLsizeiptr size, const GLvoid* data,
                         GLenum usage)
    {
        glBufferData(target, size, data, usage);
        THROW_IF_GL_ERROR();
    }

    void set_buffer_subdata(GLenum target, GLintptr offset,
                            GLsizeiptr size, const GLvoid* data)
    {
        glBufferSubData(target, offset, size, data);
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

    bool is_buffer(GLuint buffer)
    {
        return glIsBuffer(buffer) != 0;
    }

    GLsizei get_buffer_size(GLenum target)
    {
        GLint size;
        glGetBufferParameteriv(target, GL_BUFFER_SIZE, &size);
        THROW_IF_GL_ERROR();
        return size;
    }

    GLenum get_buffer_usage(GLenum target)
    {
        GLint usage;
        glGetBufferParameteriv(target, GL_BUFFER_USAGE, &usage);
        THROW_IF_GL_ERROR();
        return GLenum(usage);
    }
}
