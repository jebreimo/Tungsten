//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "Tungsten/Gl/GlTexture.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * What a texture's texels mean, which decides whether they are colour to
     * be decoded or numbers to be left exactly as they are.
     *
     * COLOR is albedo, diffuse and emissive maps: authored in sRGB and sampled
     * back as linear, so that lighting arithmetic happens in the space where
     * adding two lights means what it says.
     *
     * DATA is normal maps, roughness and specular masks, and glyph coverage.
     * The texel *is* the value; decoding it would corrupt it. A tangent-space
     * normal run through an sRGB decode is not a slightly wrong normal, it is
     * not a normal at all.
     */
    enum class TextureContent
    {
        COLOR,
        DATA
    };

    /**
     * The encoding COLOR or DATA implies for a given pixel layout.
     *
     * The mapping is not one-to-one, which is the reason callers say what a
     * texture is for rather than naming a colour space: float texels are
     * already linear, so COLOR asks for no decode there, while an 8-bit colour
     * texture does. Throws for single-channel COLOR, which GLES 3.0 has no
     * sRGB format for.
     */
    [[nodiscard]]
    ColorSpace to_color_space(TextureContent content, TextureSourceFormat format);

    /**
     * A GPU texture owned by the ResourceManager and referred to through a
     * TextureRef. It owns its GL object via the RAII TextureHandle, so it
     * is move-only — destroying the Texture (dropping it from its slot) deletes
     * the GL texture. width/height/format describe the allocated image so the
     * renderer and material layer can reason about it without round-tripping to
     * the driver.
     *
     * Sampling state is deliberately not part of the texture: `sampler` names an
     * interned SamplerDescriptor, and the renderer binds that sampler object to
     * the same unit as the texture, which overrides the texture object's own
     * glTexParameter state entirely. A null `sampler` means the manager's default
     * sampler (linear, no mipmaps, clamped), so a caller that does not care about
     * filtering need not say anything.
     */
    struct Texture
    {
        TextureHandle gl_handle;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat format = TextureFormat::RGBA;
        /**
         * What the texels mean. Recorded so the material layer can check a
         * texture against the sampler slot it is bound to; it does not change
         * how an already-uploaded texture is sampled, since that was fixed by
         * the internal format at upload. Prefer create_texture(TextureImage2D),
         * which sets this and the internal format from one answer.
         */
        TextureContent content = TextureContent::COLOR;
        SamplerRef sampler;
    };

    /**
     * A 2D image to upload, and the properties of the texture that will own
     * it. Handed to ResourceManager::create_texture, which allocates the GL
     * texture, uploads level 0, and records the description on the Texture.
     *
     * Going through this rather than uploading by hand is what keeps `content`
     * honest: the same answer picks the internal format and is stored on the
     * resource, so the two cannot drift apart.
     */
    struct TextureImage2D
    {
        Size2I size;
        /** The layout of `pixels`. Its color_space field is ignored — the
         *  encoding is decided by `content`. */
        TextureSourceFormat format = RGBA_TEXTURE;
        TextureContent content = TextureContent::COLOR;
        const void* pixels = nullptr;
        /** A null ref selects the ResourceManager's default sampler. */
        SamplerRef sampler;
    };
} // Tungsten
