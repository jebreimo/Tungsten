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
    void setUniform(GLint location, GLint val1);

    void setUniform(GLint location, GLint val1, GLint val2);

    void setUniform(GLint location, GLint val1, GLint val2, GLint val3);

    void setUniform(GLint location, GLint val1, GLint val2,
                    GLint val3, GLint val4);

    void setUniform(GLint location, float val1);

    void setUniform(GLint location, float val1, float val2);

    void setUniform(GLint location, float val1, float val2, float val3);

    void setUniform(GLint location, float val1, float val2,
                    float val3, float val4);

    void setUniform(GLint location, const Xyz::Vector2i& vec);

    void setUniform(GLint location, const Xyz::Vector3i& vec);

    void setUniform(GLint location, const Xyz::Vector4i& vec);

    void setUniform(GLint location, const Xyz::Vector2f& vec);

    void setUniform(GLint location, const Xyz::Vector3f& vec);

    void setUniform(GLint location, const Xyz::Vector4f& vec);

    void setUniform(GLint location, const Xyz::Matrix2f& mat, bool transpose = true);

    void setUniform(GLint location, const Xyz::Matrix3f& mat, bool transpose = true);

    void setUniform(GLint location, const Xyz::Matrix4f& mat, bool transpose = true);

    template <typename T>
    class Uniform
    {
    public:
        Uniform()
                : m_Location(-1)
        {}

        Uniform(GLint location)
                : m_Location(location)
        {}

        void set(const T& value)
        {
            setUniform(m_Location, value);
        }
    private:
        GLint m_Location;
    };
}
