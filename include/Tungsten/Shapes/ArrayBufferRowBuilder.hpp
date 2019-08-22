//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    class ArrayBufferRowBuilder
    {
    public:
        ArrayBufferRowBuilder(float* values, unsigned size, unsigned rowIndex);

        ArrayBufferRowBuilder& set(unsigned index, float value);

        ArrayBufferRowBuilder& set(unsigned index, float value1, float value2);

        ArrayBufferRowBuilder& set(unsigned index, float value1, float value2, float value3);

        ArrayBufferRowBuilder& set(unsigned index, const Xyz::Vector2f& value);

        ArrayBufferRowBuilder& set(unsigned index, const Xyz::Vector3f& value);

        unsigned rowIndex() const;
    private:
        float* m_Values;
        unsigned m_Size;
        unsigned m_RowIndex;
    };
}
