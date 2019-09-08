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
        return {m_Buffer.data() + offset, m_RowSize,
                unsigned(offset / m_RowSize)};
    }

    ArrayBufferRowBuilder ArrayBufferBuilder::row(int index)
    {
        auto i = unsigned(index >= 0 ? index : int(rowCount()) + index);
        auto offset = i * m_RowSize;
        if (offset >= m_Buffer.size())
            XYZ_THROW("Index is out of range: " + std::to_string(index));
        return {m_Buffer.data() + offset, m_RowSize, i};
    }

    ArrayBufferBuilder& ArrayBufferBuilder::addElement(unsigned short index)
    {
        m_Indices.push_back(index);
        return *this;
    }

    ArrayBufferBuilder& ArrayBufferBuilder::addElement(unsigned short index1,
                                                       unsigned short index2)
    {
        m_Indices.push_back(index1);
        m_Indices.push_back(index2);
        return *this;
    }

    ArrayBufferBuilder& ArrayBufferBuilder::addElement(unsigned short index1,
                                                       unsigned short index2,
                                                       unsigned short index3)
    {
        m_Indices.push_back(index1);
        m_Indices.push_back(index2);
        m_Indices.push_back(index3);
        return *this;
    }

    ArrayBufferBuilder& ArrayBufferBuilder::setElement(int elementIndex,
                                                       unsigned short rowIndex)
    {
        auto i = unsigned(elementIndex >= 0
                          ? elementIndex
                          : int(m_Indices.size()) + elementIndex);
        m_Indices[i] = rowIndex;
        return *this;
    }

    const void* ArrayBufferBuilder::arrayBuffer() const
    {
        return m_Buffer.data();
    }

    unsigned ArrayBufferBuilder::arrayBufferSize() const
    {
        return m_Buffer.size() * sizeof(float);
    }

    unsigned ArrayBufferBuilder::elementCount() const
    {
        return m_Indices.size();
    }

    const void* ArrayBufferBuilder::elementArrayBuffer() const
    {
        return m_Indices.data();
    }

    unsigned ArrayBufferBuilder::elementArrayBufferSize() const
    {
        return m_Indices.size() * sizeof(unsigned short);
    }

    void setBuffers(GLuint arrayBuffer, GLuint elementArrayBuffer,
                    const ArrayBufferBuilder& builder,
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

    void setValues(ArrayBufferBuilder& builder,
                   int firstRow,
                   unsigned columnIndex,
                   const Xyz::Vector2f& point,
                   unsigned numberOfRows)
    {
        for (unsigned i = 0; i < numberOfRows; ++i)
        {
            auto row = builder.row(int(i + firstRow));
            row.set(columnIndex, point);
        }
    }

    void setValues(ArrayBufferBuilder& builder,
                   int firstRow,
                   unsigned columnIndex,
                   const Xyz::Vector3f& point,
                   unsigned numberOfRows)
    {
        for (unsigned i = 0; i < numberOfRows; ++i)
        {
            auto row = builder.row(int(i + firstRow));
            row.set(columnIndex, point);
        }
    }
}
