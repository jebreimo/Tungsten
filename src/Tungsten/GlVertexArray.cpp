//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertexArray.hpp"
#include "Tungsten/GlError.hpp"

namespace Tungsten
{
    void GlVertexArrayDeleter::operator()(GLuint id) const
    {
        glDeleteVertexArrays(1, &id);
        THROW_IF_GL_ERROR();
    }

    VertexArrayHandle generateVertexArray()
    {
        GLuint id;
        glGenVertexArrays(1, &id);
        THROW_IF_GL_ERROR();
        return VertexArrayHandle(id);
    }

    std::vector<VertexArrayHandle> generateVertexArrays(GLsizei count)
    {
        if (count == 0)
            return std::vector<VertexArrayHandle>();

        auto ids = std::vector<GLuint>(size_t(count));
        glGenVertexArrays(count, ids.data());
        THROW_IF_GL_ERROR();
        auto result = std::vector<VertexArrayHandle>();
        for (auto id : ids)
            result.emplace_back(id);
        return result;
    }

    void bindVertexArray(GLuint vertexArray)
    {
        glBindVertexArray(vertexArray);
        THROW_IF_GL_ERROR();
    }
}
