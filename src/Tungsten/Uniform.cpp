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
    void setUniform(GLint location, GLint val1)
    {
        glUniform1i(location, val1);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, GLint val1, GLint val2)
    {
        glUniform2i(location, val1, val2);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, GLint val1, GLint val2, GLint val3)
    {
        glUniform3i(location, val1, val2, val3);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, GLint val1, GLint val2,
                    GLint val3, GLint val4)
    {
        glUniform4i(location, val1, val2, val3, val4);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, float val1)
    {
        glUniform1f(location, val1);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, float val1, float val2)
    {
        glUniform2f(location, val1, val2);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, float val1, float val2, float val3)
    {
        glUniform3f(location, val1, val2, val3);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, float val1, float val2,
                    float val3, float val4)
    {
        glUniform4f(location, val1, val2, val3, val4);
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, const Xyz::Vector2i& vec)
    {
        glUniform2iv(location, 1, vec.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, const Xyz::Vector3i& vec)
    {
        glUniform3iv(location, 1, vec.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, const Xyz::Vector4i& vec)
    {
        glUniform4iv(location, 1, vec.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, const Xyz::Vector2f& vec)
    {
        glUniform2fv(location, 1, vec.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, const Xyz::Vector3f& vec)
    {
        glUniform3fv(location, 1, vec.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, const Xyz::Vector4f& vec)
    {
        glUniform4fv(location, 1, vec.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, Xyz::Matrix2f mat, bool transpose)
    {
        if (transpose)
            Xyz::transposeInplace(mat);
        glUniformMatrix2fv(location, 1, false, mat.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, Xyz::Matrix3f mat, bool transpose)
    {
        if (transpose)
            Xyz::transposeInplace(mat);
        glUniformMatrix3fv(location, 1, false, mat.data());
        THROW_IF_GL_ERROR();
    }

    void setUniform(GLint location, Xyz::Matrix4f mat, bool transpose)
    {
        if (transpose)
            Xyz::transposeInplace(mat);
        glUniformMatrix4fv(location, 1, false, mat.data());
        THROW_IF_GL_ERROR();
    }
}
