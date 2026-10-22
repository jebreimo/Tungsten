//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Model/Model.hpp>

/**
 * Adds two copper cubes, two units wide and side by side along the root's
 * x axis, under @a parent. The left one has a normal map with a grid of
 * studs, the right one does not.
 *
 * register_builtin_shader_families must have been called on @a resources.
 */
Tungsten::Model add_bump_cube_model(Tungsten::ResourceManager& resources,
                                    Tungsten::Scene& scene,
                                    Tungsten::NodeId parent = {});
