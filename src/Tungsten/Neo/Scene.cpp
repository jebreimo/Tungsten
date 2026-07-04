//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/Scene.hpp"

#include <utility>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    Node& Scene::add(std::unique_ptr<Node> node)
    {
        if (!node)
            TUNGSTEN_THROW("Scene: node is null.");
        if (node->parent())
            TUNGSTEN_THROW("Scene: node still has a parent.");

        roots_.push_back(std::move(node));
        return *roots_.back();
    }

    std::unique_ptr<Node> Scene::remove(Node& node)
    {
        for (auto it = roots_.begin(); it != roots_.end(); ++it)
        {
            if (it->get() != &node)
                continue;

            std::unique_ptr<Node> released = std::move(*it);
            roots_.erase(it);
            return released;
        }
        TUNGSTEN_THROW("Scene: not a root of this scene.");
    }

    const std::vector<std::unique_ptr<Node>>& Scene::roots() const
    {
        return roots_;
    }

    DoubleBuffer<RenderSnapshot>& Scene::snapshots()
    {
        return snapshots_;
    }
} // Tungsten
