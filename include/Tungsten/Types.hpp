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
        UINT8,
        FLOAT
    };

    struct TextureSourceFormat
    {
        TextureFormat format;
        TextureType type;
    };

    enum class AttributeType
    {
        INT8,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        FLOAT,
        DOUBLE
    };

    enum class BufferTarget
    {
        ARRAY,
        ELEMENT_ARRAY
    };

    enum class BufferUsage
    {
        STATIC_DRAW,
        DYNAMIC_DRAW
    };

    enum class FramebufferTarget
    {
        FRAMEBUFFER,
        DRAW,
        READ
    };

    enum class FrameBufferAttachment
    {
        COLOR0,
        DEPTH,
        STENCIL
    };
}
