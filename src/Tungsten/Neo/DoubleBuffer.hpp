//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <array>

namespace Tungsten
{
    // The two-slot hand-off between the scene and the renderer (§5): the
    // SnapshotBuilder fills back(), swap() flips which slot is which, and the
    // renderer only ever reads front() — so it never observes half-built
    // state. The swap is an index flip; the slots reuse their backing storage
    // across frames rather than reallocating.
    template <typename T>
    class DoubleBuffer
    {
    public:
        [[nodiscard]]
        T& back()
        {
            return buffers_[1 - front_index_];
        }

        [[nodiscard]]
        const T& back() const
        {
            return buffers_[1 - front_index_];
        }

        [[nodiscard]]
        T& front()
        {
            return buffers_[front_index_];
        }

        [[nodiscard]]
        const T& front() const
        {
            return buffers_[front_index_];
        }

        void swap()
        {
            front_index_ = 1 - front_index_;
        }

    private:
        std::array<T, 2> buffers_;
        unsigned front_index_ = 0;
    };
} // Tungsten
