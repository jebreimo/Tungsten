//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/TransformUpdater.hpp"

#include "Tungsten/Neo/Scene.hpp"

namespace Tungsten
{
    void TransformUpdater::resolve(Scene& scene)
    {
        scene.resolve_transforms();
    }
} // Tungsten