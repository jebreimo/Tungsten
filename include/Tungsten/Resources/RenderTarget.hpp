//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Gl/GlFramebuffer.hpp"
#include "Tungsten/Gpu/GpuTypes.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * Somewhere a render pass can draw other than the window: a colour
     * texture, optionally with a depth attachment, that later passes can
     * sample.
     *
     * Owned by the ResourceManager and named by a RenderTargetRef. A null ref
     * means the window's framebuffer, so a pass that draws to the screen names
     * no target at all.
     *
     * `color` and `depth` are ordinary TextureRefs — the depth attachment is a
     * depth *texture*, not a renderbuffer, because a texture is what Metal and
     * Vulkan attach and because it can then be sampled. Both are owned by the
     * texture pool, not by this; destroying the target leaves them alone.
     *
     * The framebuffer handle is owning, so a RenderTarget is move-only and
     * destroy_render_target retires it through the DeletionQueue rather than
     * deleting it while a frame may still be reading it.
     */
    struct RenderTarget
    {
        FramebufferHandle framebuffer;
        TextureRef color;
        TextureRef depth;
        Size2I size;
    };
} // Tungsten
