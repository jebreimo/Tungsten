//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "PipelineRegistry.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    PipelineRef PipelineRegistry::register_pipeline(
        const PipelineDescriptor& descriptor)
    {
        for (size_t i = 0; i < pipelines_.size(); ++i)
        {
            if (pipelines_[i] == descriptor)
                return {static_cast<uint32_t>(i), PIPELINE_GENERATION};
        }
        pipelines_.push_back(descriptor);
        return {static_cast<uint32_t>(pipelines_.size() - 1),
                PIPELINE_GENERATION};
    }

    const PipelineDescriptor&
    PipelineRegistry::get_pipeline(PipelineRef ref) const
    {
        if (ref.generation != PIPELINE_GENERATION
            || ref.index >= pipelines_.size())
        {
            TUNGSTEN_THROW("PipelineRegistry: invalid pipeline ref.");
        }
        return pipelines_[ref.index];
    }
} // Tungsten
