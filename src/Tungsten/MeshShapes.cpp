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

    void write(std::ostream& os, const std::vector<uint16_t>& indices)
    {
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            os << i << ": "
                << indices[i] << ' '
                << indices[i + 1] << ' '
                << indices[i + 2] << '\n';
        }
    }

    void write(std::ostream& os, const VertexArrayData<PositionNormal>& buffer)
    {
        os << "Vertices:\n";
        for (size_t i = 0; i < buffer.vertices.size(); i++)
        {
            os << i << ": ";
            write(os, buffer.vertices[i]);
        }
        os << "Indices:\n";
        write(os, buffer.indices);
    }

    void write(std::ostream& os, const PositionNormalTexture& p)
    {
        os << std::get<0>(p) << " "
           << std::get<1>(p) << " "
           << std::get<2>(p) << '\n';
    }

    void write(std::ostream& os, const VertexArrayData<PositionNormalTexture>& buffer)
    {
        os << "Vertices:\n";
        for (size_t i = 0; i < buffer.vertices.size(); i++)
        {
            os << i << ": ";
            write(os, buffer.vertices[i]);
        }
        os << "Indices:\n";
        write(os, buffer.indices);
    }
}
