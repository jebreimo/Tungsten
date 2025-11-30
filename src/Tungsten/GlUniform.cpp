//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlUniform.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void set_uniform(int32_t location, int32_t val1)
    {
        get_ogl_wrapper().uniform1_i(location, val1);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, int32_t val1, int32_t val2)
    {
        get_ogl_wrapper().uniform2_i(location, val1, val2);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, int32_t val1, int32_t val2, int32_t val3)
    {
        get_ogl_wrapper().uniform3_i(location, val1, val2, val3);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, int32_t val1, int32_t val2,
                     int32_t val3, int32_t val4)
    {
        get_ogl_wrapper().uniform4_i(location, val1, val2, val3, val4);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, float val1)
    {
        get_ogl_wrapper().uniform1_f(location, val1);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, float val1, float val2)
    {
        get_ogl_wrapper().uniform2_f(location, val1, val2);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, float val1, float val2, float val3)
    {
        get_ogl_wrapper().uniform3_f(location, val1, val2, val3);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, float val1, float val2,
                     float val3, float val4)
    {
        get_ogl_wrapper().uniform4_f(location, val1, val2, val3, val4);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, const Xyz::Vector2I& vec)
    {
        get_ogl_wrapper().uniform2(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, const Xyz::Vector3I& vec)
    {
        get_ogl_wrapper().uniform3(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, const Xyz::Vector4I& vec)
    {
        get_ogl_wrapper().uniform4(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, const Xyz::Vector2F& vec)
    {
        get_ogl_wrapper().uniform2(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, const Xyz::Vector3F& vec)
    {
        get_ogl_wrapper().uniform3(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, const Xyz::Vector4F& vec)
    {
        get_ogl_wrapper().uniform4(location, 1, vec.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, Xyz::Matrix2F mat, bool transpose)
    {
        if (transpose)
            Xyz::transpose_inplace(mat);
        get_ogl_wrapper().uniform_matrix2(location, 1, false, mat.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, Xyz::Matrix3F mat, bool transpose)
    {
        if (transpose)
            Xyz::transpose_inplace(mat);
        get_ogl_wrapper().uniform_matrix3(location, 1, false, mat.values);
        THROW_IF_GL_ERROR();
    }

    void set_uniform(int32_t location, Xyz::Matrix4F mat, bool transpose)
    {
        if (transpose)
            Xyz::transpose_inplace(mat);
        get_ogl_wrapper().uniform_matrix4(location, 1, false, mat.values);
        THROW_IF_GL_ERROR();
    }
}
