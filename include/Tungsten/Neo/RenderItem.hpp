//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <array>
#include <Xyz/Matrix.hpp>

#include "ResourceRefs.hpp"

namespace Tungsten
{
    constexpr size_t RENDER_ITEM_DATA_SIZE = (4 + 3) * 4;
    
    class RenderItem
    {
    public:
        [[nodiscard]] Xyz::Matrix4F world_transform() const;

        void set_world_transform(const Xyz::Matrix4F& transform);

        [[nodiscard]] Xyz::Matrix3F normal_matrix() const;

        void set_normal_matrix(const Xyz::Matrix3F& normal_matrix);

        [[nodiscard]]  MeshRef mesh() const;

        void set_mesh(MeshRef mesh);

        [[nodiscard]] MaterialRef material() const;

        void set_material(MaterialRef material);

        [[nodiscard]] uint64_t sort_key() const;

        void set_sort_key(uint64_t sort_key);

        // The item's std140 per-draw block — u_model followed by
        // u_normal_matrix, both as column-major mat4 — ready to upload
        // verbatim to the per-draw UBO (binding 2, §4).
        [[nodiscard]]
        const std::array<float, RENDER_ITEM_DATA_SIZE>& data() const
        {
            return data_;
        }
    private:
        std::array<float, RENDER_ITEM_DATA_SIZE> data_ = {};
        MeshRef mesh_ = {};
        MaterialRef material_ = {};
        uint64_t sort_key_ = 0;
    };
} // Tungsten
