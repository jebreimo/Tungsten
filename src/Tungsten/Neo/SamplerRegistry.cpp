//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/SamplerRegistry.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    SamplerRef SamplerRegistry::register_sampler(const SamplerDescriptor& descriptor)
    {
        for (size_t i = 0; i < entries_.size(); ++i)
        {
            if (entries_[i].descriptor == descriptor)
                return {static_cast<uint32_t>(i), SAMPLER_GENERATION};
        }

        // Configuring a sampler object touches no binding state, so unlike
        // VaoCache::build_vao this needs no notify_gl_state_changed().
        auto handle = generate_sampler();
        set_sampler_parameters(handle.id(), descriptor);
        entries_.push_back({descriptor, std::move(handle)});
        return {static_cast<uint32_t>(entries_.size() - 1), SAMPLER_GENERATION};
    }

    uint32_t SamplerRegistry::get_sampler_id(SamplerRef ref)
    {
        if (!ref)
            ref = default_sampler();
        if (ref.generation != SAMPLER_GENERATION || ref.index >= entries_.size())
            TUNGSTEN_THROW("SamplerRegistry: invalid sampler ref.");
        return entries_[ref.index].handle.id();
    }

    const SamplerDescriptor& SamplerRegistry::get_descriptor(SamplerRef ref) const
    {
        if (ref.generation != SAMPLER_GENERATION || ref.index >= entries_.size())
            TUNGSTEN_THROW("SamplerRegistry: invalid sampler ref.");
        return entries_[ref.index].descriptor;
    }

    SamplerRef SamplerRegistry::default_sampler()
    {
        if (!default_sampler_)
        {
            // mip_filter must be NONE: SamplerDescriptor's own default asks for
            // mipmaps, which would leave a single-level texture — the renderer's
            // 1x1 white dummy among them — incomplete.
            default_sampler_ = register_sampler(
                {
                    .min_filter = SamplerMinMagFilter::LINEAR,
                    .mag_filter = SamplerMinMagFilter::LINEAR,
                    .mip_filter = SamplerMipFilter::NONE,
                    .address_mode_u = SamplerAddressMode::CLAMP_TO_EDGE,
                    .address_mode_v = SamplerAddressMode::CLAMP_TO_EDGE
                });
        }
        return default_sampler_;
    }
} // Tungsten
