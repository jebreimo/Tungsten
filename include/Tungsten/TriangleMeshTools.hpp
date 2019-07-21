//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-20.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>
#include <Xyz/Meshes/TriangleMesh.hpp>

namespace Tungsten
{
    struct ArrayContents
    {
        enum Type
        {
            POINTS = 0,
            NORMALS = 1,
            TEXTURES = 2,
            DEFINED = 4,
            ALL = POINTS + NORMALS + TEXTURES,
            ALL_DEFINED = ALL + DEFINED
        };
    };

    void setElementArrayBuffer(GLuint bufferId,
                               size_t faceCount,
                               const Xyz::TriangleFace* faces,
                               GLenum usage = GL_STATIC_DRAW);

    void setBuffers(GLuint arrayBuffer,
                    GLuint elementArrayBuffer,
                    const Xyz::TriangleMesh<float>& mesh,
                    GLenum usage = GL_STATIC_DRAW,
                    ArrayContents::Type contents = ArrayContents::POINTS);
}
