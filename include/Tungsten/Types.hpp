//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-15.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    struct Size2D
    {
        GLsizei width;
        GLsizei height;
    };

    template <typename T>
    Xyz::Vector<T, 2> to_vector2(const Size2D& size)
    {
        return {T(size.width), T(size.height)};
    }

    struct Position2D
    {
        GLint x;
        GLint y;
    };

    template <typename T>
    Xyz::Vector<T, 2> to_vector2(const Position2D& size)
    {
        return {T(size.x), T(size.y)};
    }

    struct TextureSourceFormat
    {
        GLenum format;
        GLenum type;
    };
}
