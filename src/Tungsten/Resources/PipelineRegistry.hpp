//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <vector>
#include "Tungsten/Resources/PipelineDescriptor.hpp"
#include "Tungsten/Resources/ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * Interns the distinct PipelineDescriptors an application uses and hands
     * them out as PipelineRef. Owned by ResourceManager.
     *
     * Like LayoutRegistry and SamplerRegistry, and for the same reasons, this
     * is deliberately not a GenerationalPool: a pipeline owns no GL object of
     * its own and is never individually freed, so there is no free-list, no
     * per-slot generation and no deletion path. The set of pipelines an
     * application uses is small and bounded, and a permanently stable ref can
     * be packed into a sort key without any revocation concern.
     *
     * Holding a ShaderProgramRef indefinitely is safe because ResourceManager
     * exposes no way to destroy a shader: a compiled variant lives as long as
     * the manager does, so a pipeline can never name a program that has gone
     * away. Re-registering a shader family does invalidate the *variant cache*
     * without disturbing programs already handed out, so a hot-reload has to
     * re-register its pipelines too, not merely re-resolve its materials.
     */
    class PipelineRegistry
    {
    public:
        static constexpr uint32_t PIPELINE_GENERATION = 1;

        /**
         * Returns the ref for a pipeline equal to `descriptor`, interning it
         * first if it has not been seen.
         */
        PipelineRef register_pipeline(const PipelineDescriptor& descriptor);

        /**
         * Resolves a ref to the interned descriptor, validating it so a null
         * or garbage ref throws rather than aliasing.
         */
        [[nodiscard]]
        const PipelineDescriptor& get_pipeline(PipelineRef ref) const;

    private:
        std::vector<PipelineDescriptor> pipelines_;
    };
} // Tungsten
