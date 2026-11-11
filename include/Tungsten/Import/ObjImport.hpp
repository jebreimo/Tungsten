//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "../Model/Model.hpp"
#include "ImportedWireframe.hpp"

namespace Tungsten
{
    /**
     * Settings for import_obj.
     */
    struct ObjImportOptions
    {
        /**
         * Whether to also build a line-segment version of every imported
         * group of faces, returned in ObjImport::wireframes.
         */
        bool create_wireframes = false;
    };

    /**
     * What import_obj added to the scene: the Model, and what is particular
     * to Wavefront OBJ.
     *
     * The model's root is a new child of the node the caller passed as the
     * parent, and its bounds are never empty: import_obj throws instead. An
     * OBJ file has no animations, so the model has none.
     */
    struct ObjImport : Model
    {
        /** The number of face groups that became renderables. */
        uint32_t primitive_count = 0;

        /** One entry per imported renderable if
         *  ObjImportOptions::create_wireframes was set, otherwise empty. */
        std::vector<ImportedWireframe> wireframes;

        /** One line per problem the file or its materials had. Worth showing
         *  to the user: an otherwise successful import can still be missing
         *  parts of the file. */
        std::vector<std::string> warnings;
    };

    /**
     * @brief Builds a node hierarchy for a Wavefront OBJ file under @a parent.
     *
     * The faces of each shape (`o` or `g`) in the file become one renderable
     * per material they use, each on its own child node of the model's root.
     * Polygons are triangulated; lines and points are skipped with a warning. Materials are read from the file's MTL libraries, which are
     * looked for next to the OBJ file.
     *
     * Everything is set up to be drawn by the builtin Blinn-Phong shader
     * family, so register_builtin_shader_families must have been called on
     * @a resources first. An MTL material maps onto it directly: Kd, Ks, Ns
     * and d, with map_Kd, map_Ks and norm as its textures. map_Bump is used
     * as a normal map if there is no norm, unless its image has a single
     * channel, which marks it as a height map. Ka is ignored, as exporters
     * fill it in too inconsistently to be useful; the ambient color is
     * derived from Kd. Faces without normals are drawn flat-shaded.
     *
     * @throw TungstenException if the file cannot be read or parsed, or if it
     *      contains nothing that can be displayed. Neither the scene nor
     *      @a resources keeps anything from a failed import.
     */
    ObjImport import_obj(const std::filesystem::path& file_name,
                         ResourceManager& resources,
                         Scene& scene,
                         NodeId parent = {},
                         const ObjImportOptions& options = {});
} // Tungsten
