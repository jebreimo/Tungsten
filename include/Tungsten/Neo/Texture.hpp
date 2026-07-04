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

namespace Tungsten
{
    // A GPU texture owned by the ResourceManager and referred to through a
    // TextureRef (§6). It owns its GL object via the RAII TextureHandle, so it
    // is move-only — destroying the Texture (dropping it from its slot) deletes
    // the GL texture. width/height/format describe the allocated image so the
    // renderer and material layer can reason about it without round-tripping to
    // the driver.
    struct Texture
    {
        TextureHandle gl_handle;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat format = TextureFormat::RGBA;
    };
} // Tungsten