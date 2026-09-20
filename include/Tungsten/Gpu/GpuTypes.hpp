//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-15.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <Xyz/Vector.hpp>
#include "../Detail/GenericBitmaskOperators.hpp"

/**
 * @file
 * The graphics vocabulary shared by every layer above the backend.
 *
 * Nothing here names an OpenGL type or constant: these are the neutral
 * descriptions of formats, buffers, samplers and fixed-function state that the
 * resource, scene-graph and rendering layers speak. Translating them into GL
 * enums is the backend's job and happens in one place,
 * src/Tungsten/Gl/GlTypeConversion.hpp.
 */

namespace Tungsten
{
    using Size2I = Xyz::Vector2I;
    using Position2I = Xyz::Vector2I;

    enum class TextureFormat
    {
        R,
        RGB,
        RGBA,
        /**
         * A depth buffer. Only meaningful as a render target's depth
         * attachment; there is nothing to upload into one.
         */
        DEPTH
    };

    enum class TextureValueType
    {
        UINT8,
        /** 32-bit unsigned, used for depth attachments. */
        UINT32,
        FLOAT
    };

    /**
     * How the texels handed to the GL are encoded.
     *
     * SRGB texels are decoded to linear by the sampler, so filtering and
     * mipmap generation average linear values rather than encoded ones. Only
     * 8-bit RGB and RGBA can be stored this way.
     */
    enum class ColorSpace
    {
        LINEAR,
        SRGB
    };

    struct TextureSourceFormat
    {
        TextureFormat format;
        TextureValueType type;
        /**
         * The encoding of the data being uploaded, which selects the texture's
         * internal format. It is a property of the allocation, so
         * set_texture_sub_image_2d ignores it — a sub-image inherits the
         * encoding the level was created with.
         */
        ColorSpace color_space = ColorSpace::LINEAR;
    };

    enum class BufferTarget
    {
        ARRAY,
        ELEMENT_ARRAY,
        COPY_READ,
        COPY_WRITE,
        PIXEL_PACK,
        PIXEL_UNPACK,
        UNIFORM,
        TRANSFORM_FEEDBACK
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
        NONE,
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

    /**
     * Note when porting: TRIANGLE_FAN and LINE_LOOP have no Metal equivalent.
     * Neither is used by the scene-graph path — both reach the GL only through
     * the draw_polygon_* convenience wrappers in Gl/GlRendering.hpp.
     */
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

    enum class VertexAttributeDataType : uint8_t
    {
        NONE,
        INT32,
        UINT32,
        FLOAT
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

    enum class SamplerMipFilter
    {
        NONE,
        NEAREST,
        LINEAR
    };

    enum class SamplerMinMagFilter
    {
        NEAREST,
        LINEAR
    };

    enum class SamplerAddressMode
    {
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE
    };

    enum class SamplerCompareFunction
    {
        NONE,
        LEQUAL,
        GEQUAL,
        LESS,
        GREATER,
        EQUAL,
        NOTEQUAL,
        ALWAYS,
        NEVER
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

    enum class FaceCullingMode
    {
        FRONT,
        BACK,
        FRONT_AND_BACK
    };

    /**
     * How a fragment's depth is compared against the depth buffer.
     *
     * Distinct from SamplerCompareFunction, which has a NONE member meaning
     * "this sampler does no comparison at all". A depth test that is not
     * wanted is turned off with DepthState::test, not spelled here.
     */
    enum class CompareFunction
    {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS
    };

    /**
     * How a blend's source and destination terms are combined, once each has
     * been scaled by its BlendFunction. All five are core in GLES 3.0.
     */
    enum class BlendEquation
    {
        ADD,
        SUBTRACT,
        REVERSE_SUBTRACT,
        MIN,
        MAX
    };

    /**
     * Which winding a front face has. The scene's meshes are wound
     * counter-clockwise, which is also the GL default.
     */
    enum class FrontFace
    {
        COUNTER_CLOCKWISE,
        CLOCKWISE
    };

    /**
     * Which colour channels a draw is allowed to write.
     */
    enum class ColorWriteMask : uint8_t
    {
        NONE = 0,
        RED = 1,
        GREEN = 2,
        BLUE = 4,
        ALPHA = 8,
        RGB = RED | GREEN | BLUE,
        ALL = RGB | ALPHA
    };

    TUNGSTEN_ENABLE_BITMASK_OPERATORS(ColorWriteMask);
}
