//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-15.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
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

    enum class TextureValueType
    {
        UINT8,
        FLOAT
    };

    struct TextureSourceFormat
    {
        TextureFormat format;
        TextureValueType type;
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

    enum class TextureTarget
    {
        TEXTURE_2D,
        CUBE_MAP,
        TEXTURE_3D,
        ARRAY_2D
    };

    enum class TextureTarget2D
    {
        TEXTURE_2D,
        CUBE_MAP_POSITIVE_X,
        CUBE_MAP_POSITIVE_Y,
        CUBE_MAP_POSITIVE_Z,
        CUBE_MAP_NEGATIVE_X,
        CUBE_MAP_NEGATIVE_Y,
        CUBE_MAP_NEGATIVE_Z,
    };

    enum class TextureParameter
    {
        BASE_LEVEL,
        MAX_LEVEL,
        MIN_FILTER,
        MAG_FILTER,
        WRAP_S,
        WRAP_T,
        WRAP_R,
        COMPARE_MODE,
        COMPARE_FUNC,
        MIN_LOD,
        MAX_LOD,
        SWIZZLE_R,
        SWIZZLE_G,
        SWIZZLE_B,
        SWIZZLE_A,
    };

    enum class TopologyType
    {
        POINTS,
        LINES,
        LINE_STRIP,
        LINE_LOOP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN
    };

    enum class ShaderType
    {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    enum class VertexAttributeType
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

    enum class ElementIndexType
    {
        UINT8,
        UINT16,
        UINT32
    };

    enum class FramebufferStatus
    {
        COMPLETE,
        UNDEFINED,
        INCOMPLETE_ATTACHMENT,
        INCOMPLETE_MISSING_ATTACHMENT,
        UNSUPPORTED,
        INCOMPLETE_MULTISAMPLE
    };

    enum class TextureMinFilter
    {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR
    };

    enum class TextureMagFilter
    {
        NEAREST,
        LINEAR
    };

    enum class TextureWrapMode
    {
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE
    };

    enum class BlendFunction
    {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_SATURATE
    };

    enum class CullMode
    {
        FRONT,
        BACK,
        FRONT_AND_BACK
    };
}
