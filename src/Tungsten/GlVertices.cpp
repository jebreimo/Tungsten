//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertices.hpp"

#include "GlTypeConversion.h"
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

    void draw_array(TopologyType topology, int32_t offset, int32_t count)
    {
        glDrawArrays(to_ogl_draw_mode(topology), int32_t(offset), count);
    }

    void draw_elements(TopologyType topology, GLenum type, int32_t offset, int32_t count)
    {
        intptr_t tmp_offset = offset * get_type_size(type);
        glDrawElements(to_ogl_draw_mode(topology), count, type, reinterpret_cast<void*>(tmp_offset));
        THROW_IF_GL_ERROR();
    }

    void draw_elements_16(TopologyType topology, int32_t offset, int32_t count)
    {
        draw_elements(topology, GL_UNSIGNED_SHORT, offset, count);
    }
}
