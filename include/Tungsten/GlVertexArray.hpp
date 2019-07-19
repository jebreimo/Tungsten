//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlVertexArrayDeleter
    {
        void operator()(GLuint id) const;
    };

    using VertexArrayHandle = GlHandle<GlVertexArrayDeleter>;

    VertexArrayHandle generateVertexArray();

    std::vector<VertexArrayHandle> generateVertexArrays(GLsizei count);

    void bindVertexArray(GLuint vertexArray);
}
