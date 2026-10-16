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

    class CoordinateSystem
    {
    public:
        CoordinateSystem(ResourceManager& resource_manager,
                         Scene& scene,
                         BufferArenaRef vbo_arena = {},
                         BufferArenaRef ebo_arena = {});

        ~CoordinateSystem();

        [[nodiscard]] NodeHandle hub() const;
    private:
        ResourceManager& resources_;
        BufferArenaRef vbo_arena_ = {};
        BufferArenaRef ebo_arena_ = {};
        NodeHandle hub_;
    };
} // Tungsten
