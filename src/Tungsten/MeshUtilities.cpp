//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/MeshUtilities.hpp"

#include <set>

namespace Tungsten
{
    void write_triangles(std::ostream& os,
                         std::span<const uint16_t> indices,
                         bool indexes)
    {
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            if (indexes)
                os << i << ": ";
            os << indices[i] << ' '
                << indices[i + 1] << ' '
                << indices[i + 2] << '\n';
        }
    }

    void write_line_segments(std::ostream& os,
                             std::span<const uint16_t> indices,
                             bool indexes)
    {
        for (size_t i = 0; i < indices.size(); i += 2)
        {
            if (indexes)
                os << i << ": ";
            os << indices[i] << ' '
                << indices[i + 1] << '\n';
        }
    }

    std::vector<uint16_t>
    triangles_to_line_segments(std::span<const uint16_t> triangle_indices)
    {
        auto cmp = [](std::pair<uint16_t, uint16_t> a,
                      std::pair<uint16_t, uint16_t> b)
        {
            if (a.first > a.second)
                std::swap(a.first, a.second);
            if (b.first > b.second)
                std::swap(b.first, b.second);
            if (a.first != b.first)
                return a.first < b.first;
            return a.second < b.second;
        };

        std::set<std::pair<uint16_t, uint16_t>, decltype(cmp)> edges(cmp);
        for (size_t i = 0; i < triangle_indices.size(); i += 3)
        {
            edges.emplace(triangle_indices[i], triangle_indices[i + 1]);
            edges.emplace(triangle_indices[i + 1], triangle_indices[i + 2]);
            edges.emplace(triangle_indices[i + 2], triangle_indices[i]);
        }

        std::vector<uint16_t> line_indices;
        line_indices.reserve(edges.size() * 2);
        for (const auto& edge : edges)
        {
            line_indices.push_back(edge.first);
            line_indices.push_back(edge.second);
        }
        return line_indices;
    }
}
