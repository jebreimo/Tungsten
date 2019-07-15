//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertices.hpp"
#include "Tungsten/GlError.hpp"


namespace Tungsten
{
    GLsizei getTypeSize(GLenum type)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            return sizeof(unsigned short);
        case GL_UNSIGNED_BYTE:
            return 1;
        default:
            GL_THROW("Unsupported type: " + std::to_string(type));
        }
    }

    void drawElements(GLenum mode, GLsizei count, GLenum type, GLsizei offset)
    {
        auto tmpOffset = size_t(offset * getTypeSize(type));
        glDrawElements(mode, count, type, reinterpret_cast<void*>(tmpOffset));
        THROW_IF_GL_ERROR();
    }

    void enableVertexAttribute(GLuint location)
    {
        glEnableVertexAttribArray(location);
        THROW_IF_GL_ERROR();
    }

    void disableVertexAttribute(GLuint location)
    {
        glDisableVertexAttribArray(location);
        THROW_IF_GL_ERROR();
    }

    void defineVertexAttributePointer(GLuint attributeLocation,
                                      GLint size,
                                      GLenum type,
                                      bool normalized,
                                      GLsizei stride,
                                      size_t offset)
    {
        glVertexAttribPointer(attributeLocation, size, type,
                              GLboolean(normalized ? 1 : 0),
                              stride,
                              reinterpret_cast<void*>(offset));
        THROW_IF_GL_ERROR();
    }
}
