//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <vector>
#include "DoubleBuffer.hpp"
#include "Node.hpp"
#include "RenderSnapshot.hpp"

namespace Tungsten
{
    // The mutable scene graph: owns the root nodes (each the top of an owned
    // subtree) and the double-buffered snapshots the renderer reads (§5).
    // Roots are held through unique_ptr for the same pointer-stability reason
    // as Node::children (§2).
    class Scene
    {
    public:
        // Takes ownership of node as a new root and returns a reference to
        // it. Throws if node is null or still attached to a parent.
        Node& add(std::unique_ptr<Node> node);

        // Detaches a root and returns ownership of it, so the caller can
        // attach it below another node or drop it. Throws if node is not a
        // root of this scene.
        std::unique_ptr<Node> remove(Node& node);

        [[nodiscard]]
        const std::vector<std::unique_ptr<Node>>& roots() const;

        [[nodiscard]]
        DoubleBuffer<RenderSnapshot>& snapshots();

    private:
        std::vector<std::unique_ptr<Node>> roots_;
        DoubleBuffer<RenderSnapshot> snapshots_;
    };
} // Tungsten
