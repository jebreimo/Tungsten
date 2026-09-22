//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <Xyz/BBox.hpp>
#include "../Resources/ResourceManager.hpp"
#include "../SceneGraph/Scene.hpp"

namespace Tungsten
{
    /**
     * Settings for import_gltf.
     */
    struct GltfImportOptions
    {
        /**
         * The index of the scene to import. An empty value means the file's
         * default scene, or its first scene if it names no default.
         */
        std::optional<size_t> scene_index;
    };

    /**
     * What import_gltf added to the scene.
     */
    struct GltfImport
    {
        /** The root of the imported hierarchy, a new child of the node the
         *  caller passed as the parent. */
        NodeId root;

        /** The union of every imported renderable's bounds, in the root's
         *  coordinate system. Never empty: import_gltf throws instead. */
        Xyz::BBox3F bounds;

        /** The number of primitives that became renderables. */
        uint32_t primitive_count = 0;

        /** One line per primitive or feature that was skipped. Worth showing
         *  to the user: an otherwise successful import can still be missing
         *  parts of the file. */
        std::vector<std::string> warnings;
    };

    /**
     * @brief Builds a node hierarchy for a glTF file's scene under @a parent.
     *
     * Creates the meshes, materials and textures the scene needs in
     * @a resources. Reads both glTF and GLB files. Everything is set up to be
     * drawn by the builtin Blinn-Phong shader family, so
     * register_builtin_shader_families must have been called on @a resources
     * first, and the file's metallic-roughness materials are approximated
     * rather than reproduced.
     *
     * Skips what it cannot draw — non-triangle primitives, positionless
     * primitives — recording a line in GltfImport::warnings for each, and
     * throws only if nothing drawable is left.
     *
     * @throw TungstenException if the file cannot be read or parsed, or if the
     *      scene contains nothing that can be displayed.
     */
    GltfImport import_gltf(const std::filesystem::path& file_name,
                           ResourceManager& resources,
                           Scene& scene,
                           NodeId parent = {},
                           const GltfImportOptions& options = {});
} // Tungsten
