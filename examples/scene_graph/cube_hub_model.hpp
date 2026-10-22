//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Model/Model.hpp>

/** How far along the hub's x axis the child cubes are placed. */
constexpr float CHILD_CUBE_OFFSET = 2.2f;

/** The scale of the child cubes relative to the hub. */
constexpr float CHILD_CUBE_SCALE = 0.6f;

/**
 * A model with a cube at its root and two smaller child cubes.
 */
struct CubeHubModel
{
    /** The model. Its root is the hub cube. */
    Tungsten::Model model;

    /** The transparent child cube, at -CHILD_CUBE_OFFSET. */
    Tungsten::NodeId left;

    /** The opaque child cube, at CHILD_CUBE_OFFSET. */
    Tungsten::NodeId right;
};

/**
 * Adds a gold cube, two units wide and centered on its root, under
 * @a parent, with a transparent copper cube to its left and a slate cube
 * to its right as child nodes.
 *
 * register_builtin_shader_families must have been called on @a resources.
 */
CubeHubModel add_cube_hub_model(Tungsten::ResourceManager& resources,
                                Tungsten::Scene& scene,
                                Tungsten::NodeId parent = {});
