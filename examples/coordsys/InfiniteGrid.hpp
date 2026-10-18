//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-14.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Resources/BuiltinShaders.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/SceneGraph/NodeHandle.hpp"

namespace Tungsten
{
    constexpr ShaderFamilyId GRID_FAMILY = FIRST_USER_SHADER_FAMILY;
    constexpr ShaderFamilyId AXIS_FAMILY = FIRST_USER_SHADER_FAMILY + 1;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;

    class InfiniteGridResources
    {
    public:
        InfiniteGridResources(ResourceManager& resource_manager,
                              BufferArenaRef vbo_arena = {},
                              BufferArenaRef ebo_arena = {});

        ~InfiniteGridResources();

        [[nodiscard]] MeshRef square_mesh() const
        {
            return square_mesh_;
        }

        [[nodiscard]] MaterialRef grid_material() const
        {
            return grid_material_;
        }

        [[nodiscard]] MaterialRef axis_material() const
        {
            return axis_material_;
        }

    private:
        ResourceManager& resources_;
        BufferArenaRef vbo_arena_ = {};
        BufferArenaRef ebo_arena_ = {};
        MeshRef square_mesh_ = {};
        MaterialRef grid_material_ = {};
        MaterialRef axis_material_ = {};
    };

    NodeHandle
    make_infinite_grid_node(Scene& scene, const InfiniteGridResources& resources);
} // Tungsten
