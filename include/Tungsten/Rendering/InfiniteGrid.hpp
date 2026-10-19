//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-14.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>
#include "Tungsten/Resources/ResourceRefs.hpp"
#include "Tungsten/SceneGraph/NodeHandle.hpp"

namespace Tungsten
{
    class ResourceManager;
    class Scene;

    /**
     * The appearance of an InfiniteGrid. Colours are sRGB, the numbers a
     * colour picker shows; widths are fractions of a grid cell.
     */
    struct InfiniteGridStyle
    {
        Xyz::Vector4F line_color = {0.85f, 0.85f, 0.85f, 1.0f};
        Xyz::Vector4F base_color = {0.16f, 0.17f, 0.19f, 1.0f};
        Xyz::Vector4F x_axis_color = {0.90f, 0.22f, 0.25f, 1.0f};
        Xyz::Vector4F y_axis_color = {0.35f, 0.80f, 0.30f, 1.0f};
        Xyz::Vector4F z_axis_color = {0.25f, 0.45f, 0.95f, 1.0f};
        float line_width = 0.01f;
        float axis_width = 0.05f;
        float cells_per_unit = 1.0f;
        bool show_y_axis = true;
    };

    /**
     * An infinite grid on the world's xz plane, with the x and z axes drawn
     * as highlighted grid lines and the y axis as a line going up from the
     * origin. The grid is drawn with the "pristine grid" shader from Ben
     * Golus' article The Best Darn Grid Shader (Yet).
     *
     * The grid ignores its nodes' transforms: it always lies in the world's
     * xz plane and reaches past the far plane of whichever camera views it.
     *
     * Owns the mesh and materials the grid's nodes refer to, so it must
     * outlive every node added by add_to(). The constructor creates GL
     * objects, so a context must be current.
     */
    class InfiniteGrid
    {
    public:
        /**
         * Registers the grid's shader families and creates its mesh and
         * materials.
         */
        explicit InfiniteGrid(ResourceManager& resources,
                              const InfiniteGridStyle& style = {});

        ~InfiniteGrid();

        InfiniteGrid(const InfiniteGrid&) = delete;

        InfiniteGrid& operator=(const InfiniteGrid&) = delete;

        /**
         * Adds the grid to @a scene as a node with one renderable child per
         * part of the grid, and returns the node.
         */
        NodeHandle add_to(Scene& scene) const;

    private:
        ResourceManager& resources_;
        BufferArenaRef vbo_arena_ = {};
        BufferArenaRef ebo_arena_ = {};
        MeshRef square_mesh_ = {};
        MaterialRef grid_material_ = {};
        MaterialRef axis_material_ = {};
    };
} // Tungsten
