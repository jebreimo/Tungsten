//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "ResourceRefs.hpp"
#include "SharedBuffer.hpp"

namespace Tungsten
{
    /**
     * Everything one load unit — a model, a level, a screen — created in a
     * ResourceManager, so that it can be unloaded with one call to
     * ResourceManager::destroy_bundle.
     *
     * The bundle owns what it lists; the refs elsewhere (scene components,
     * snapshots) stay weak. Interned resources — layouts, samplers, pipelines,
     * shader variants — are shared by design and never belong in a bundle.
     *
     * The meshes are views: destroying the bundle does not free their slices
     * one mesh at a time. The slices are freed from `slices` instead, exactly
     * once each, so several meshes can share one, and a slice in one of
     * `arenas` need not be listed at all, as the whole arena goes with the
     * bundle.
     */
    struct ResourceBundle
    {
        std::vector<MeshRef> meshes;
        std::vector<MaterialRef> materials;
        std::vector<TextureRef> textures;
        std::vector<RenderTargetRef> render_targets;
        std::vector<SharedBuffer> slices;
        std::vector<BufferArenaRef> arenas;

        [[nodiscard]]
        bool empty() const
        {
            return meshes.empty() && materials.empty() && textures.empty()
                   && render_targets.empty() && slices.empty()
                   && arenas.empty();
        }
    };
}
