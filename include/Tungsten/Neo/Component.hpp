//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

namespace Tungsten
{
    class Node;

    // Base class for the data that gives a Node meaning: renderables, cameras,
    // lights. A component belongs to exactly one node for its whole life;
    // Node::add_component takes ownership and sets the owner back-pointer.
    class Component
    {
    public:
        virtual ~Component() = default;

        // The node this component belongs to, or nullptr until the component
        // has been added to one.
        [[nodiscard]]
        Node* owner() const
        {
            return owner_;
        }

    private:
        friend class Node;

        Node* owner_ = nullptr;
    };
} // Tungsten
