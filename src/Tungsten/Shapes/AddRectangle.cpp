//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Shapes/AddRectangle.hpp"
#include <Xyz/Xyz.hpp>
#include <Xyz/Geometry/CoordinateSystem.hpp>
#include "Tungsten/GlError.hpp"

namespace Tungsten
{
    void addRectangle(ArrayBufferBuilder& builder,
                      const Xyz::Rectangle<float>& rect,
                      const Xyz::CoordinateSystem<float>& cs,
                      float offset)
    {
        builder.reserve(4);
        for (size_t i = 0; i < 4; ++i)
        {
            auto row = builder.addRow();
            row.set(0, cs.toWorldPos(makeVector3(rect.point(i), offset)));
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
        setValues(builder, normal, builder.rowCount() - 4, 4, 3);
    }

    void setPoints(ArrayBufferBuilder& builder,
                   unsigned firstRow, unsigned columnIndex,
                   const Xyz::Rectangle<float>& rect)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            auto row = builder.row(firstRow + i);
            row.set(columnIndex, rect.point(i));
        }
    }

    //void addRectangle(ArrayBufferBuilder& builder,
    //                  const Xyz::Rectangle<float>& rect,
    //                  const Xyz::CoordinateSystem<float>& cs,
    //                  const Xyz::Rectangle<float>& textureRect,
    //                  float offset)
    //{
    //    if (builder.rowSize() < 5)
    //        GL_THROW("Texture coordinates require a row size of at least 5 floats.");
    //    size_t index;
    //    if (builder.rowSize() >= 8)
    //    {
    //        addRectangleWithNormals(builder, rect, cs, offset);
    //        index = 6;
    //    }
    //    else
    //    {
    //        addRectangleWithoutNormals(builder, rect, cs, offset);
    //        index = 3;
    //    }
    //    setPoints(builder, builder.rowCount() - 4, index, textureRect);
    //}
}
