//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-08-14.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include <Xyz/Xyz.hpp>
#include <Xyz/Geometry/CoordinateSystem.hpp>
#include "ArrayBufferBuilder.hpp"

namespace Tungsten
{
    void addRectangle(ArrayBufferBuilder& builder,
                      const Xyz::Rectangle<float>& rect,
                      const Xyz::CoordinateSystem<float>& cs,
                      float offset = 0);

    void addRectangleWithNormals(ArrayBufferBuilder& builder,
                                 const Xyz::Rectangle<float>& rect,
                                 const Xyz::CoordinateSystem<float>& cs,
                                 float offset = 0);

    void setPoints(ArrayBufferBuilder& builder,
                   unsigned firstRow,
                   unsigned columnIndex,
                   const Xyz::Rectangle<float>& rect);
}
