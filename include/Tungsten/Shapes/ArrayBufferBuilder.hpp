//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include <GL/glew.h>
#include "ArrayBufferRowBuilder.hpp"

namespace Tungsten
{
    class ArrayBufferBuilder
    {
    public:
        explicit ArrayBufferBuilder(unsigned rowSize, unsigned rowCount = 0);

        unsigned rowSize() const;

        unsigned rowCount() const;

        void reserve(unsigned rowCount);

        void resize(unsigned rowCount);

        ArrayBufferRowBuilder addRow();

        ArrayBufferRowBuilder row(int index);

        unsigned elementCount() const;

        void addElement(unsigned short index);

        void addElement(unsigned short index1,
                        unsigned short index2);

        void addElement(unsigned short index1,
                        unsigned short index2,
                        unsigned short index3);

        void setElement(int elementIndex,
                        unsigned short rowIndex);

        const void* arrayBuffer() const;

        unsigned arrayBufferSize() const;

        const void* elementArrayBuffer() const;

        unsigned elementArrayBufferSize() const;
    private:
        std::vector<float> m_Buffer;
        std::vector<unsigned short> m_Indices;
        unsigned m_RowSize;
    };

    void setBuffers(GLuint arrayBuffer,
                    GLuint elementArrayBuffer,
                    const ArrayBufferBuilder& mesh,
                    GLenum usage = GL_STATIC_DRAW);

    void setValues(ArrayBufferBuilder& builder,
                   const Xyz::Vector2f& point,
                   unsigned firstRow,
                   unsigned numberOfRows,
                   unsigned columnIndex);

    void setValues(ArrayBufferBuilder& builder,
                   const Xyz::Vector3f& point,
                   unsigned firstRow,
                   unsigned numberOfRows,
                   unsigned columnIndex);
}
