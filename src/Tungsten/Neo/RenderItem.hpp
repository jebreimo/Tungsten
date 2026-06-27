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

namespace Tungsten
{
    class RenderItem
    {
    public:
        [[nodiscard]] Xyz::Matrix4F worldTransform() const
        {
            Xyz::Matrix4F result;
            for (size_t i = 0; i < 16; ++i)
                result[i % 4, i / 4] = data_[i];
            return result;
        }

        void set_world_transform(const Xyz::Matrix4F& transform)
        {
            for (size_t i = 0; i < 16; ++i)
                data_[i] = transform[i % 4, i / 4];
        }

        void set_normal_matrix(const Xyz::Matrix3F& normal_matrix)
        {
            for (size_t i = 0; i < 9; ++i)
                data_[16 + 4 * (i / 3) + i % 3] = normal_matrix[i % 3, i / 3];
        }

        [[nodiscard]] Xyz::Matrix3F normalMatrix() const
        {
            // Data is using the std140 layout, so the normal matrix is stored
            // in a 4x4 matrix with the last row and column unused.
            Xyz::Matrix3F result;
            for (size_t i = 0; i < 9; ++i)
                result[i % 3, i / 3] = data_[16 + 4 * (i / 3) + i % 3];
            return result;
        }
    private:
        std::array<float, 32> data_ = {};
    };
} // Tungsten
