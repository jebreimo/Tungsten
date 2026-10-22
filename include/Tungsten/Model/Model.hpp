//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <vector>
#include <Xyz/BBox.hpp>
#include "../Animation/AnimationPlayer.hpp"
#include "../Resources/ResourceBundle.hpp"
#include "../Resources/ResourceManager.hpp"
#include "../SceneGraph/Scene.hpp"

namespace Tungsten
{
    /**
     * A node hierarchy in a scene together with the resources only it uses
     * and the animations that move it: one unit to show, animate and unload.
     *
     * Built with a ModelBuilder, or by an importer such as import_gltf.
     * Unload it with remove_model.
     */
    struct Model
    {
        /** The root of the model's hierarchy. */
        NodeId root;

        /** The union of every renderable's bounds, in the root's coordinate
         *  system, with skinned meshes in their bind pose. */
        Xyz::BBox3F bounds;

        /** The model's animations. Play one with an AnimationPlayer made
         *  from it and `targets`. */
        std::vector<std::shared_ptr<const AnimationClip>> animations;

        /** The nodes the animations' channels target, and their transforms
         *  at rest. */
        AnimationTargets targets;

        /** Every mesh, material, texture and arena created for the model. */
        ResourceBundle resources;
    };

    /**
     * Removes the model's nodes from @a scene and destroys its resources,
     * leaving @a model empty. Does nothing to a model that is already empty.
     */
    void remove_model(Model& model, Scene& scene, ResourceManager& resources);
} // Tungsten
