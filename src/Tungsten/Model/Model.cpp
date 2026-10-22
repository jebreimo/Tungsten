//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Model/Model.hpp"

namespace Tungsten
{
    void remove_model(Model& model, Scene& scene, ResourceManager& resources)
    {
        if (model.root && scene.is_alive(model.root))
            scene.remove(model.root);
        resources.destroy_bundle(model.resources);
        model = {};
    }
}
