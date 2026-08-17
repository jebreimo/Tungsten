//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ResourceRef.hpp"

namespace Tungsten
{
    /**
     * A phantom tag, like the resource tags in ResourceRefs.hpp: it gives
     * NodeId its own type so a node reference cannot be passed where a mesh or
     * material reference is expected.
     */
    struct NodeTag;

    /**
     * A revocable reference to a node in a Scene. Nodes live in the
     * scene's flat arrays and have no address of their own, so this
     * {index, generation} pair is the only way to name one. Removing a node
     * bumps its generation, which makes every outstanding id to it fail
     * validation instead of silently naming whichever node reuses the slot.
     *
     * Generation 0 is reserved for the null id, so a default-constructed
     * NodeId is null and never resolves — which is also how "no parent" is
     * spelled.
     */
    using NodeId = ResourceRef<NodeTag>;
} // Tungsten
