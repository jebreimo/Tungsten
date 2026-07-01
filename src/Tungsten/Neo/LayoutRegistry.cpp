//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/LayoutRegistry.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    VertexLayoutRef LayoutRegistry::register_layout(const VertexLayout& layout)
    {
        for (size_t i = 0; i < layouts_.size(); ++i)
        {
            if (layouts_[i] == layout)
                return {static_cast<uint32_t>(i), LAYOUT_GENERATION};
        }
        layouts_.push_back(layout);
        return {static_cast<uint32_t>(layouts_.size() - 1), LAYOUT_GENERATION};
    }

    const VertexLayout& LayoutRegistry::get_layout(VertexLayoutRef ref) const
    {
        if (ref.generation != LAYOUT_GENERATION || ref.index >= layouts_.size())
            TUNGSTEN_THROW("LayoutRegistry: invalid layout ref.");
        return layouts_[ref.index];
    }
} // Tungsten