//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ChildIterator.hpp"
#include "NodeId.hpp"

namespace Tungsten
{
    /**
     * A range of a node's children, iterable with a range-based for loop.
     */
    class ChildRange
    {
    public:
        ChildRange(Scene* scene, NodeId first)
            : scene_(scene), first_(first)
        {
        }

        [[nodiscard]]
        ChildIterator begin() const
        {
            return {scene_, first_};
        }

        [[nodiscard]]
        ChildIterator end() const
        {
            return {scene_, NodeId{}};
        }

        [[nodiscard]]
        bool empty() const
        {
            return !first_;
        }

    private:
        Scene* scene_;
        NodeId first_;
    };
}
