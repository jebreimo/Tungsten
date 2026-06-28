//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GlStateCache.hpp"

#include "UboBindings.hpp"
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlProgram.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Gl/GlVertexArray.hpp"

namespace Tungsten
{
    void GlStateCache::use_program(uint32_t program_id)
    {
        if (program_id == current_program_)
            return;
        // Qualified to call the free wrapper, not this method.
        Tungsten::use_program(program_id);
        current_program_ = program_id;
    }

    void GlStateCache::bind_vao(uint32_t vao_id)
    {
        if (vao_id == current_vao_)
            return;
        bind_vertex_array(vao_id);
        current_vao_ = vao_id;
    }

    void GlStateCache::bind_texture(int32_t unit, uint32_t texture_id)
    {
        const auto it = bound_textures_.find(unit);
        if (it != bound_textures_.end() && it->second == texture_id)
            return;
        activate_texture_unit(unit);
        // Qualified to call the free wrapper, not this method.
        Tungsten::bind_texture(TextureTarget::TEXTURE_2D, texture_id);
        bound_textures_[unit] = texture_id;
    }

    void GlStateCache::bind_material_ubo(uint32_t buffer_id)
    {
        if (buffer_id == current_material_ubo_)
            return;
        bind_buffer_base(BufferTarget::UNIFORM, PER_MATERIAL_UBO_BINDING,
                         buffer_id);
        current_material_ubo_ = buffer_id;
    }

    void GlStateCache::invalidate()
    {
        current_program_ = 0;
        current_vao_ = 0;
        current_material_ubo_ = 0;
        bound_textures_.clear();
    }
} // Tungsten