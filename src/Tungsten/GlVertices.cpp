//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertices.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        int32_t get_type_size(GLenum type)
        {
            switch (type)
            {
            case GL_UNSIGNED_SHORT:
                return sizeof(unsigned short);
            case GL_UNSIGNED_BYTE:
                return 1;
            default:
                TUNGSTEN_THROW("Unsupported type: " + std::to_string(type));
            }
        }
    }

    void draw_array(GLenum mode, int32_t offset, int32_t count)
    {
        glDrawArrays(mode, int32_t(offset), count);
    }

    void draw_elements(GLenum mode, GLenum type, int32_t offset, int32_t count)
    {
        intptr_t tmp_offset = offset * get_type_size(type);
        glDrawElements(mode, count, type, reinterpret_cast<void*>(tmp_offset));
        THROW_IF_GL_ERROR();
    }

    void draw_elements_16(GLenum mode, int32_t offset, int32_t count)
    {
        draw_elements(mode, GL_UNSIGNED_SHORT, offset, count);
    }
}
