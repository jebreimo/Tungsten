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
#include <Xyz/Rectangle.hpp>

#include "ResourceRefs.hpp"

namespace Tungsten
{
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

        [[nodiscard]] Xyz::RectangleF bounds() const;

        void set_bounds(const Xyz::RectangleF& bounds);
    private:
        std::array<float, 32> data_ = {};
        MeshRef mesh_ = {};
        MaterialRef material_ = {};
        uint64_t sort_key_ = 0;
        Xyz::RectangleF bounds_;
    };
} // Tungsten
