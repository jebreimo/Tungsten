//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "PositionNormalShapes.hpp"

namespace Tungsten
{
    std::ostream& operator<<(std::ostream& os, const PositionNormal& p)
    {
        return os << std::get<0>(p) << " " << std::get<1>(p);
    }

    std::ostream& operator<<(std::ostream& os, const VertexArrayData<PositionNormal>& buffer)
    {
        os << "Vertexes:\n";
        for (const auto& v : buffer.vertexes)
            os << v << '\n';
        os << "Indexes:";
        for (size_t i = 0; i < buffer.indexes.size(); i += 3)
        {
            os << '\n' << buffer.indexes[i] << ' '
                << buffer.indexes[i + 1] << ' '
                << buffer.indexes[i + 2];
        }
        return os;
    }

    void add_rect(VertexArrayDataBuilder<PositionNormal>& builder,
                  const Xyz::Rectangle3F& rect)
    {
        const auto base_index = builder.max_index();
        const auto normal = rect.normal_vector();
        for (int i = 0; i < 4; ++i)
            builder.add_vertex({rect[i], normal});

        builder.add_indexes(base_index, base_index + 1, base_index + 2);
        builder.add_indexes(base_index, base_index + 2, base_index + 3);
    }

    void add_cube(VertexArrayDataBuilder<PositionNormal>& builder)
    {
        constexpr auto PI = Xyz::Constants<float>::PI;
        using O3F = Xyz::Orientation3F;
        const auto size = Xyz::Vector2F(2, 2);
        add_rect(builder, {{{-1, -1, 1}, O3F{0, 0, 0}}, size});
        add_rect(builder, {{{-1, -1, 1}, O3F{PI / 2, 0, -PI / 2}}, size});
        add_rect(builder, {{{-1, -1, -1}, O3F{0, 0, PI / 2}}, size});
        add_rect(builder, {{{-1, 1, -1}, O3F{0, 0, PI}}, size});
        add_rect(builder, {{{1, -1, -1}, O3F{PI / 2, 0, PI / 2}}, size});
        add_rect(builder, {{{-1, 1, 1}, O3F{0, 0, -PI / 2}}, size});
    }
}
