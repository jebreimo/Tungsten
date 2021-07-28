//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Shapes/AddRectangle.hpp"
#include <Xyz/Xyz.hpp>
#include <Xyz/CoordinateSystem.hpp>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void addRectangle(ArrayBufferBuilder& builder,
                      const Xyz::Rectangle<float>& rect,
                      float offset)
    {
        auto index = builder.rowCount();
        builder.reserve(index + 4);
        for (size_t i = 0; i < 4; ++i)
        {
            auto row = builder.addRow();
            row.set(0, makeVector3(rect.vertex(i), offset));
        }
        builder.addElement(index, index + 1, index + 2);
        builder.addElement(index, index + 2, index + 3);
    }

    void addRectangle(ArrayBufferBuilder& builder,
                      const Xyz::Rectangle<float>& rect,
                      const Xyz::CoordinateSystem<float>& cs,
                      float offset)
    {
        builder.reserve(builder.rowCount() + 4);
        for (size_t i = 0; i < 4; ++i)
        {
            auto row = builder.addRow();
            row.set(0, cs.toWorldPos(makeVector3(rect.vertex(i), offset)));
        }
        auto index = builder.rowCount() - 4;
        builder.addElement(index, index + 1, index + 2);
        builder.addElement(index, index + 2, index + 3);
    }

    void addRectangleWithNormals(ArrayBufferBuilder& builder,
                                 const Xyz::Rectangle<float>& rect,
                                 const Xyz::CoordinateSystem<float>& cs,
                                 float offset)
    {
        addRectangle(builder, rect, cs, offset);
        auto normal = getUnit(cs.fromWorldTransform().row(2));
        if (rect.isClockwise())
            normal = -normal;
        setValues(builder, -4, 3, normal, 4);
    }

    void setPoints(ArrayBufferBuilder& builder,
                   int firstRow, unsigned columnIndex,
                   const Xyz::Rectangle<float>& rect)
    {
        for (int i = 0; i < 4; ++i)
        {
            auto row = builder.row(firstRow + i);
            row.set(columnIndex, rect.vertex(i));
        }
    }
}
