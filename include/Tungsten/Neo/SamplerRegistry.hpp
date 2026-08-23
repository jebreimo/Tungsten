//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <vector>
#include "Tungsten/Gl/GlSampler.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * Interns the distinct SamplerDescriptors an application uses, creates one
     * GL sampler object per distinct descriptor, and hands them out as
     * SamplerRef. Owned by ResourceManager.
     *
     * A sampler object holds the filtering, wrapping and LOD state that would
     * otherwise live in each texture object, and overrides that state entirely
     * while bound to a texture unit. Keeping it here rather than on the texture
     * means Neo describes sampling declaratively — a Texture names a descriptor,
     * the renderer binds the object — instead of every caller baking parameters
     * into a GL texture before handing it over.
     *
     * Like LayoutRegistry, and unlike the GenerationalPools, this is a plain
     * interning vector: the set of distinct descriptors an application uses is
     * small and bounded, and a sampler is never individually freed, so there is
     * no free-list, no per-slot generation, and no deletion path. Refs never go
     * stale, so their generation carries no information; every valid ref uses the
     * fixed SAMPLER_GENERATION and only the index identifies the sampler.
     * Generation 0 is left as the null ref, matching the other refs.
     *
     * The one way it differs from LayoutRegistry: a sampler does own a GL object.
     * Each entry keeps its SamplerHandle, so the whole set is deleted when the
     * registry dies. Nothing is ever retired into the DeletionQueue, because
     * nothing is ever taken out of service while frames are in flight.
     */
    class SamplerRegistry
    {
    public:
        static constexpr uint32_t SAMPLER_GENERATION = 1;

        /**
         * Returns the ref for a sampler equal to `descriptor`, creating the GL
         * sampler object and interning it first if it has not been seen.
         * Equality is SamplerDescriptor's defaulted operator==.
         */
        SamplerRef register_sampler(const SamplerDescriptor& descriptor);

        /**
         * Resolves a ref to the GL id to bind. A null ref resolves to the
         * default sampler, so a Texture that names no sampler still gets one.
         */
        [[nodiscard]] uint32_t get_sampler_id(SamplerRef ref);

        /**
         * Resolves a ref to the interned descriptor, validating it so a null or
         * garbage ref throws rather than aliasing.
         */
        [[nodiscard]] const SamplerDescriptor& get_descriptor(SamplerRef ref) const;

        /**
         * The sampler a Texture with a null ref is drawn with: linear filtering,
         * no mipmaps, clamped.
         *
         * Interned on the first call rather than in the constructor, because
         * creating the GL object needs a live context and ResourceManager must
         * stay constructible before one exists.
         */
        SamplerRef default_sampler();

    private:
        struct Entry
        {
            SamplerDescriptor descriptor;
            SamplerHandle handle;
        };

        std::vector<Entry> entries_;
        SamplerRef default_sampler_;
    };
} // Tungsten
