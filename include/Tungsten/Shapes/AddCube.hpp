//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-08-16.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ArrayBufferBuilder.hpp"

namespace Tungsten
{
    enum ArrayBufferContents
    {
        POINTS = 0,
        NORMALS = 1,
        TEXTURES = 2,
        STANDARD = NORMALS | TEXTURES
    };

    void addCube(ArrayBufferBuilder& builder, unsigned contents = STANDARD);
}
