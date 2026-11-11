//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "../Resources/ResourceRefs.hpp"
#include "../SceneGraph/NodeId.hpp"

namespace Tungsten
{
    /**
     * The wireframe alternative to one imported renderable: assign the mesh
     * and the material to the node's RenderableComponent to draw the edges of
     * its triangles instead of their surface.
     *
     * The mesh shares its vertices with the renderable's own mesh. Both are
     * in the import's resources, which own those vertices, so unload them
     * with remove_model rather than with destroy_mesh.
     */
    struct ImportedWireframe
    {
        NodeId node;
        MeshRef mesh;
        MaterialRef material;
    };
} // Tungsten
