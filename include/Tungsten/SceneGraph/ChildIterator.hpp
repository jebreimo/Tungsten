//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "NodeId.hpp"

namespace Tungsten
{
    class Scene;
    class NodeHandle;

    /**
     * Iterates a node's children through the sibling chain. Yields NodeIds
     * and allocates nothing.
     */
    class ChildIterator
    {
    public:
        ChildIterator() = default;

        ChildIterator(Scene* scene, NodeId id)
            : scene_(scene), id_(id)
        {
        }

        NodeHandle operator*() const;

        ChildIterator& operator++();

        bool operator==(const ChildIterator& other) const
        {
            return id_ == other.id_;
        }

    private:
        Scene* scene_ = nullptr;
        NodeId id_;
    };
}
