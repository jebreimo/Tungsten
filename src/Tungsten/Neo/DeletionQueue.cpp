//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/DeletionQueue.hpp"
#include <algorithm>

namespace Tungsten
{
    void DeletionQueue::begin_frame(uint64_t frame)
    {
        current_frame_ = frame;
    }

    void DeletionQueue::retire(RetiredHandle handle)
    {
        entries_.push_back({std::move(handle), current_frame_});
    }

    void DeletionQueue::collect_garbage(uint64_t completed_frame)
    {
        // Dropping an entry destroys its owning handle, which runs the GL delete.
        std::erase_if(entries_, [&](const RetiredObject& entry) {
            return entry.frame <= completed_frame;
        });
    }
} // Tungsten