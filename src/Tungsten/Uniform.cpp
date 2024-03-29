//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Uniform.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void set_uniform(GLint location, GLint val1)
    {
        glUniform1i(location, val1);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, GLint val1, GLint val2)
    {
        glUniform2i(location, val1, val2);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, GLint val1, GLint val2, GLint val3)
    {
        glUniform3i(location, val1, val2, val3);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, GLint val1, GLint val2,
                     GLint val3, GLint val4)
    {
        glUniform4i(location, val1, val2, val3, val4);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, float val1)
    {
        glUniform1f(location, val1);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, float val1, float val2)
    {
        glUniform2f(location, val1, val2);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, float val1, float val2, float val3)
    {
        glUniform3f(location, val1, val2, val3);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, float val1, float val2,
                     float val3, float val4)
    {
        glUniform4f(location, val1, val2, val3, val4);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, const Xyz::Vector2I& vec)
    {
        glUniform2iv(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, const Xyz::Vector3I& vec)
    {
        glUniform3iv(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, const Xyz::Vector4I& vec)
    {
        glUniform4iv(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, const Xyz::Vector2F& vec)
    {
        glUniform2fv(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, const Xyz::Vector3F& vec)
    {
        glUniform3fv(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, const Xyz::Vector4F& vec)
    {
        glUniform4fv(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, Xyz::Matrix2F mat, bool transpose)
    {
        if (transpose)
            Xyz::transpose_inplace(mat);
        glUniformMatrix2fv(location, 1, false, mat.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, Xyz::Matrix3F mat, bool transpose)
    {
        if (transpose)
            Xyz::transpose_inplace(mat);
        glUniformMatrix3fv(location, 1, false, mat.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(GLint location, Xyz::Matrix4F mat, bool transpose)
    {
        if (transpose)
            Xyz::transpose_inplace(mat);
        glUniformMatrix4fv(location, 1, false, mat.values);
        THROW_IF_GL_ERROR();
    }
}
