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
    using Size2I = Xyz::Vector2I;
    using Position2I = Xyz::Vector2I;

    enum class TextureFormat
    {
        R,
        RGB,
        RGBA
    };

    enum class TextureType
    {
        UNSIGNED_BYTE,
        FLOAT
    };

    struct TextureSourceFormat
    {
        GLenum format;
        GLenum type;
    };
}
