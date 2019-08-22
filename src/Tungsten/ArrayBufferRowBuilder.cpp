//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Shapes/ArrayBufferRowBuilder.hpp"
#include "Tungsten/GlError.hpp"

namespace Tungsten
{
    ArrayBufferRowBuilder::ArrayBufferRowBuilder(float* values,
                                                 unsigned size,
                                                 unsigned rowIndex)
        : m_Values(values),
          m_Size(size),
          m_RowIndex(rowIndex)
    {}

    ArrayBufferRowBuilder& ArrayBufferRowBuilder::set(unsigned index, float value)
    {
        if (index + 1 > m_Size)
            GL_THROW("Assignment beyond the end of the row.");
        m_Values[index] = value;
        return *this;
    }

    ArrayBufferRowBuilder& ArrayBufferRowBuilder::set(unsigned index, float value1, float value2)
    {
        if (index + 2 > m_Size)
            GL_THROW("Assignment beyond the end of the row.");
        m_Values[index] = value1;
        m_Values[index + 1] = value2;
        return *this;
    }

    ArrayBufferRowBuilder&
    ArrayBufferRowBuilder::set(unsigned index, float value1, float value2, float value3)
    {
        if (index + 3 > m_Size)
            GL_THROW("Assignment beyond the end of the row.");
        m_Values[index] = value1;
        m_Values[index + 1] = value2;
        m_Values[index + 2] = value3;
        return *this;
    }

    ArrayBufferRowBuilder& ArrayBufferRowBuilder::set(unsigned index, const Xyz::Vector2f& value)
    {
        return set(index, value[0], value[1]);
    }

    ArrayBufferRowBuilder& ArrayBufferRowBuilder::set(unsigned index, const Xyz::Vector3f& value)
    {
        return set(index, value[0], value[1], value[2]);
    }

    unsigned ArrayBufferRowBuilder::rowIndex() const
    {
        return m_RowIndex;
    }
}
