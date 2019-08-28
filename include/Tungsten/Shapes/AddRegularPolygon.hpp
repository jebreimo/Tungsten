//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-08-28.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include "Tungsten/Shapes/ArrayBufferBuilder.hpp"

namespace Tungsten
{
    void addHollowRegularPolygon(ArrayBufferBuilder& builder,
                                 unsigned numberOfCorners,
                                 float innerRadius, float outerRadius,
                                 float zValue = 0);
}
