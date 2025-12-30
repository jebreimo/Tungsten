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
}
