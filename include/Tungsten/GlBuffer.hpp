//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlBufferDeleter
    {
        void operator()(GLuint id) const;
    };

    using BufferHandle = GlHandle<GlBufferDeleter>;

    void bindBuffer(GLenum target, GLuint buffer);

    BufferHandle generateBuffer();

    std::vector<BufferHandle> generateBuffers(GLsizei count);

    void setBufferData(GLenum target, GLsizeiptr size,
                       const GLvoid* data, GLenum usage);

    struct GlVertexArrayDeleter
    {
        void operator()(GLuint id) const;
    };

    using VertexArrayHandle = GlHandle<GlVertexArrayDeleter>;

    VertexArrayHandle generateVertexArray();

    std::vector<VertexArrayHandle> generateVertexArrays(GLsizei count);

    void bindVertexArray(GLuint vertexArray);
}
