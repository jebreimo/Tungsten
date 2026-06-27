//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "RenderItem.hpp"

namespace Tungsten
{
    Xyz::Matrix4F RenderItem::world_transform() const
    {
        Xyz::Matrix4F result;
        for (size_t i = 0; i < 16; ++i)
            result[i % 4, i / 4] = data_[i];
        return result;
    }

    void RenderItem::set_world_transform(const Xyz::Matrix4F& transform)
    {
        for (size_t i = 0; i < 16; ++i)
            data_[i] = transform[i % 4, i / 4];
    }

    void RenderItem::set_normal_matrix(const Xyz::Matrix3F& normal_matrix)
    {
        for (size_t i = 0; i < 9; ++i)
            data_[16 + 4 * (i / 3) + i % 3] = normal_matrix[i % 3, i / 3];
    }

    Xyz::Matrix3F RenderItem::normal_matrix() const
    {
        // Data is using the std140 layout, so the normal matrix is stored
        // in a 4x4 matrix with the last row and column unused.
        Xyz::Matrix3F result;
        for (size_t i = 0; i < 9; ++i)
            result[i % 3, i / 3] = data_[16 + 4 * (i / 3) + i % 3];
        return result;
    }

    MeshRef RenderItem::mesh() const
    {
        return mesh_;
    }

    void RenderItem::set_mesh(MeshRef mesh)
    {
        mesh_ = mesh;
    }

    MaterialRef RenderItem::material() const
    {
        return material_;
    }

    void RenderItem::set_material(MaterialRef material)
    {
        material_ = material;
    }

    uint64_t RenderItem::sort_key() const
    {
        return sort_key_;
    }

    void RenderItem::set_sort_key(uint64_t sort_key)
    {
        sort_key_ = sort_key;
    }

    Xyz::RectangleF RenderItem::bounds() const
    {
        return bounds_;
    }

    void RenderItem::set_bounds(const Xyz::RectangleF& bounds)
    {
        bounds_ = bounds;
    }
} // Tungsten
