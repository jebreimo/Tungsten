//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Shapes/ArrayBufferBuilder.hpp"
#include "Tungsten/GlBuffer.hpp"

namespace Tungsten
{
    ArrayBufferBuilder::ArrayBufferBuilder(unsigned rowSize,
                                           unsigned rowCount)
        : m_RowSize(rowSize),
          m_Buffer()
    {}

    unsigned ArrayBufferBuilder::rowSize() const
    {
        return m_RowSize;
    }

    unsigned ArrayBufferBuilder::rowCount() const
    {
        return unsigned(m_Buffer.size() / m_RowSize);
    }

    void ArrayBufferBuilder::reserve(unsigned rowCount)
    {
        m_Buffer.reserve(rowCount * m_RowSize);
    }

    void ArrayBufferBuilder::resize(unsigned rowCount)
    {
        m_Buffer.resize(rowCount * m_RowSize);
    }

    ArrayBufferRowBuilder ArrayBufferBuilder::addRow()
    {
        auto offset = m_Buffer.size();
        m_Buffer.resize(m_Buffer.size() + m_RowSize);
        return {m_Buffer.data() + offset, m_RowSize, unsigned(offset / m_RowSize)};
    }

    ArrayBufferRowBuilder ArrayBufferBuilder::row(unsigned index)
    {
        auto offset = index * m_RowSize;
        return {m_Buffer.data() + offset, m_RowSize, index};
    }

    void ArrayBufferBuilder::addElement(unsigned short index)
    {
        m_Indices.push_back(index);
    }

    void ArrayBufferBuilder::addElement(unsigned short index1,
                                        unsigned short index2)
    {
        m_Indices.push_back(index1);
        m_Indices.push_back(index2);
    }

    void ArrayBufferBuilder::addElement(unsigned short index1,
                                        unsigned short index2,
                                        unsigned short index3)
    {
        m_Indices.push_back(index1);
        m_Indices.push_back(index2);
        m_Indices.push_back(index3);
    }

    const void* ArrayBufferBuilder::arrayBuffer() const
    {
        return m_Buffer.data();
    }

    unsigned ArrayBufferBuilder::arrayBufferSize() const
    {
        return m_Buffer.size() * sizeof(float);
    }

    const void* ArrayBufferBuilder::elementArrayBuffer() const
    {
        return m_Indices.data();
    }

    unsigned ArrayBufferBuilder::elementArrayBufferSize() const
    {
        return m_Indices.size() * sizeof(unsigned short);
    }

    void setBuffers(GLuint arrayBuffer, GLuint elementArrayBuffer, const ArrayBufferBuilder& builder,
                    GLenum usage)
    {
        Tungsten::bindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
        Tungsten::setBufferData(GL_ARRAY_BUFFER,
                                builder.arrayBufferSize(),
                                builder.arrayBuffer(),
                                usage);
        Tungsten::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
        Tungsten::setBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                builder.elementArrayBufferSize(),
                                builder.elementArrayBuffer(),
                                usage);
    }

    void setValues(ArrayBufferBuilder& builder, const Xyz::Vector2f& point, unsigned firstRow,
                   unsigned numberOfRows, unsigned columnIndex)
    {
        for (unsigned i = 0; i < numberOfRows; ++i)
        {
            auto row = builder.row(i + firstRow);
            row.set(columnIndex, point);
        }
    }

    void setValues(ArrayBufferBuilder& builder, const Xyz::Vector3f& point, unsigned firstRow,
                   unsigned numberOfRows, unsigned columnIndex)
    {
        for (unsigned i = 0; i < numberOfRows; ++i)
        {
            auto row = builder.row(i + firstRow);
            row.set(columnIndex, point);
        }
    }
}
