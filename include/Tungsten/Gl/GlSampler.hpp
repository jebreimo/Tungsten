//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>

#include "GlHandle.hpp"
#include "GlTypes.hpp"

namespace Tungsten
{
    struct SamplerDeleter
    {
        void operator()(uint32_t id) const;
    };

    using SamplerHandle = GlHandle<SamplerDeleter>;

    SamplerHandle generate_sampler();

    void generate_samplers(std::span<SamplerHandle> samplers);

    void bind_sampler(uint32_t unit, uint32_t sampler);

    [[nodiscard]] uint32_t bound_sampler(uint32_t unit);

    struct SamplerDescriptor
    {
        SamplerMinMagFilter min_filter = SamplerMinMagFilter::NEAREST;
        SamplerMinMagFilter mag_filter = SamplerMinMagFilter::LINEAR;
        SamplerMipFilter mip_filter = SamplerMipFilter::LINEAR;
        SamplerAddressMode address_mode_u = SamplerAddressMode::REPEAT;
        SamplerAddressMode address_mode_v = SamplerAddressMode::REPEAT;
        SamplerAddressMode address_mode_w = SamplerAddressMode::REPEAT;
        float min_lod = -1000.f;
        float max_lod = 1000.f;
        SamplerCompareFunction compare_function = SamplerCompareFunction::NONE;

        bool operator==(const SamplerDescriptor&) const = default;
    };

    void set_sampler_parameters(uint32_t sampler, const SamplerDescriptor& descriptor);
}
