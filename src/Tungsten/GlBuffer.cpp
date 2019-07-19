//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlBuffer.hpp"
#include "Tungsten/GlError.hpp"

namespace Tungsten
{
    void GlBufferDeleter::operator()(GLuint id) const
    {
        glDeleteBuffers(1, &id);
        THROW_IF_GL_ERROR();
    }

    void bindBuffer(GLenum target, GLuint buffer)
    {
        glBindBuffer(target, buffer);
        THROW_IF_GL_ERROR();
    }

    BufferHandle generateBuffer()
    {
        GLuint id;
        glGenBuffers(1, &id);
        THROW_IF_GL_ERROR();
        return BufferHandle(id);
    }

    std::vector<BufferHandle> generateBuffers(GLsizei count)
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

    void setBufferData(GLenum target, GLsizeiptr size, const GLvoid* data,
                       GLenum usage)
    {
        glBufferData(target, size, data, usage);
        THROW_IF_GL_ERROR();
    }
}
