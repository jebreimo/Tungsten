//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-08-16.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Shapes/AddCube.hpp"
#include "Tungsten/Shapes/AddRectangle.hpp"

namespace Tungsten
{
    void addCube(ArrayBufferBuilder& builder, unsigned contents)
    {
        using CS = Xyz::CoordinateSystem<float>;
        static CS XYZ_SYSTEM({0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1});
        static CS YZX_SYSTEM({0, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 0});
        static CS ZXY_SYSTEM({0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 1, 0});
        static const CS* systems[] = {&XYZ_SYSTEM, &YZX_SYSTEM, &ZXY_SYSTEM};
        builder.reserve(24);

        Xyz::Rectangle<float> up({-1, -1}, {2, 2});
        Xyz::Rectangle<float> texUp({0, 0}, {1 / 3.0f, -0.5f});
        for (unsigned i = 0; i < 3; ++i)
        {
            if (contents & NORMALS)
                addRectangleWithNormals(builder, up, *systems[i], 1);
            else
                addRectangle(builder, up, *systems[i], 1);
            if (contents & TEXTURES)
            {
                texUp.setOrigin({float(i) / 3.0f, 0.5f});
                setPoints(builder, builder.rowCount() - 4, 6, texUp);
            }
        }
        Xyz::Rectangle<float> down({1, -1}, {-2, 2});
        Xyz::Rectangle<float> texDown({0, 0}, {-1 / 3.0f, -0.5f});
        for (unsigned i = 0; i < 3; ++i)
        {
            if (contents & NORMALS)
                addRectangleWithNormals(builder, down, *systems[i], -1);
            else
                addRectangle(builder, down, *systems[i], -1);
            if (contents & TEXTURES)
            {
                texUp.setOrigin({i / 3.0f, 1.0f});
                setPoints(builder, builder.rowCount() - 4, 6, texDown);
            }
        }
    }
}
