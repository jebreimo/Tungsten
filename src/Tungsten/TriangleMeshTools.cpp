//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-20.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlBuffer.hpp"
#include "Tungsten/GlError.hpp"
#include "Tungsten/TriangleMeshTools.hpp"

namespace Tungsten
{
    void setElementArrayBuffer(
            GLuint bufferId, size_t faceCount,
            const Xyz::TriangleFace* faces, GLenum usage)
    {
        setElementArrayBuffer(
                bufferId,
                faceCount * 3,
                reinterpret_cast<const uint16_t*>(faces),
                usage);
    }

    void setBuffers(GLuint arrayBuffer,
                    GLuint elementArrayBuffer,
                    const Xyz::TriangleMesh<float>& mesh,
                    GLenum usage,
                    ArrayContents::Type contents)
    {
        bool includeNormals = false;
        if (contents & ArrayContents::NORMALS)
        {
            if (!mesh.normals().empty())
                includeNormals = true;
            else if (!(contents & ArrayContents::DEFINED))
                GL_THROW("Mesh doesn't contain normals.");
        }
        bool includeTextures = false;
        if (contents & ArrayContents::TEXTURES)
        {
            if (!mesh.texturePoints().empty())
                includeTextures = true;
            else if (!(contents & ArrayContents::DEFINED))
                GL_THROW("Mesh doesn't contain texture points.");
        }

        auto rowSize = 3 + (includeNormals ? 3 : 0) + (includeTextures ? 2 : 0);
        std::vector<float> result(mesh.points().size() * rowSize);
        auto offset = copyPoints(mesh.points(), result.data(), result.size(), rowSize, 0);
        if (includeNormals)
            offset = copyPoints(mesh.normals(), result.data(), result.size(), rowSize, offset);
        if (includeTextures)
            copyPoints(mesh.texturePoints(), result.data(), result.size(), rowSize, offset);
        Tungsten::bindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
        Tungsten::setBufferData(GL_ARRAY_BUFFER,
                                mesh.points().size() * rowSize * sizeof(float),
                                result.data(),
                                usage);
        setElementArrayBuffer(elementArrayBuffer, mesh.faces().size(),
                              mesh.faces().data(), usage);
    }
}
