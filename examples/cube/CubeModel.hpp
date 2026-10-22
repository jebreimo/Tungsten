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
 * Adds a gold cube, two units wide and centered on its root, under
 * @a parent. Each face shows a number from one to six.
 *
 * register_builtin_shader_families must have been called on @a resources.
 */
Tungsten::Model add_cube_model(Tungsten::ResourceManager& resources,
                               Tungsten::Scene& scene,
                               Tungsten::NodeId parent = {});
