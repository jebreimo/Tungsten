//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/MeshShapes.hpp"

namespace Tungsten
{
    void write(std::ostream& os, const PositionNormal& p)
    {
        os << std::get<0>(p) << " " << std::get<1>(p) << '\n';
    }

    void write(std::ostream& os, const std::vector<uint16_t>& indexes)
    {
        for (size_t i = 0; i < indexes.size(); i += 3)
        {
            os << indexes[i] << ' '
                << indexes[i + 1] << ' '
                << indexes[i + 2] << '\n';
        }
    }

    void write(std::ostream& os, const VertexArrayData<PositionNormal>& buffer)
    {
        os << "Vertexes:\n";
        for (const auto& v : buffer.vertexes)
            write(os, v);
        os << "Indexes:\n";
        write(os, buffer.indexes);
    }

    void write(std::ostream& os, const PositionNormalTexture& p)
    {
        os << std::get<0>(p) << " "
           << std::get<1>(p) << " "
           << std::get<2>(p) << '\n';
    }

    void write(std::ostream& os, const VertexArrayData<PositionNormalTexture>& buffer)
    {
        os << "Vertexes:\n";
        for (const auto& v : buffer.vertexes)
            write(os, v);
        os << "Indexes:\n";
        write(os, buffer.indexes);
    }
}
