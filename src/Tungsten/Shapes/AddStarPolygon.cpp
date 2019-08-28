//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-08-22.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <Xyz/Xyz.hpp>
#include "Tungsten/Shapes/AddStarPolygon.hpp"

namespace Tungsten
{
    void addHollowStarPolygon(ArrayBufferBuilder& builder,
                              unsigned numberOfCorners,
                              float innerRadius, float outerRadius,
                              float zValue)
    {
        auto outer = Xyz::RegularPolygonGenerator()
                .setRadius(outerRadius)
                .setNumberOfPoints(numberOfCorners)
                .generate<float>();
        auto inner = Xyz::RegularPolygonGenerator()
                .setRadius(innerRadius)
                .setNumberOfPoints(numberOfCorners)
                .setAngle(-Xyz::PI_64 / numberOfCorners)
                .generate<float>();
        if (outer.empty())
            return; // TODO: Throw exception.
        builder.reserve(2 * numberOfCorners);
        auto index = builder.elementCount();
        for (unsigned i = 0; i < numberOfCorners; ++i)
        {
            builder.addRow().set(0, inner[i]).set(2, zValue);
            builder.addRow().set(0, outer[i]).set(2, zValue);
            auto j = index + i * 2;
            builder.addElement(j, j + 1, j + 2);
        }
        builder.setElement(-1, index);
    }
}
