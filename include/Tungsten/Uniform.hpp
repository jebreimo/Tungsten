//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>
#include <Xyz/Xyz.hpp>

namespace Tungsten
{
    void set_uniform(GLint location, GLint val1);

    void set_uniform(GLint location, GLint val1, GLint val2);

    void set_uniform(GLint location, GLint val1, GLint val2, GLint val3);

    void set_uniform(GLint location, GLint val1, GLint val2,
                     GLint val3, GLint val4);

    void set_uniform(GLint location, float val1);

    void set_uniform(GLint location, float val1, float val2);

    void set_uniform(GLint location, float val1, float val2, float val3);

    void set_uniform(GLint location, float val1, float val2,
                     float val3, float val4);

    void set_uniform(GLint location, const Xyz::Vector2i& vec);

    void set_uniform(GLint location, const Xyz::Vector3i& vec);

    void set_uniform(GLint location, const Xyz::Vector4i& vec);

    void set_uniform(GLint location, const Xyz::Vector2f& vec);

    void set_uniform(GLint location, const Xyz::Vector3f& vec);

    void set_uniform(GLint location, const Xyz::Vector4f& vec);

    void set_uniform(GLint location, Xyz::Matrix2f mat, bool transpose = true);

    void set_uniform(GLint location, Xyz::Matrix3f mat, bool transpose = true);

    void set_uniform(GLint location, Xyz::Matrix4f mat, bool transpose = true);

    template <typename T>
    class Uniform
    {
    public:
        Uniform()
                : m_location(-1)
        {}

        explicit Uniform(GLint location)
                : m_location(location)
        {}

        void set(const T& value)
        {
            set_uniform(m_location, value);
        }
    private:
        GLint m_location;
    };
}
