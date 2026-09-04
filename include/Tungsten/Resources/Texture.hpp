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
        SamplerRef sampler;
    };
} // Tungsten
