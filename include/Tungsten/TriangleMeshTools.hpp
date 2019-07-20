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

/*
        m_Buffers = Tungsten::generateBuffers(2);
        Tungsten::bindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
        Tungsten::setBufferData(
                GL_ARRAY_BUFFER,
                starPolygon.points().size() * sizeof(Xyz::Vector3f),
                starPolygon.points().data(),
                GL_STATIC_DRAW);
        Tungsten::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
        Tungsten::setBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                starPolygon.faces().size() * sizeof(Xyz::TriangleFace),
                                starPolygon.faces().data(),
                                GL_STATIC_DRAW);

 */
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

    void setBuffers(GLuint arrayBuffer,
                    GLuint elementArrayBuffer,
                    const Xyz::TriangleMesh<float>& mesh,
                    GLenum usage,
                    ArrayContents::Type contents = ArrayContents::DEFINED);
}
