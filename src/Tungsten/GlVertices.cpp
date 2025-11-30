//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertices.hpp"

#include "GlTypeConversion.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        int32_t get_type_size(ElementIndexType type)
        {
            switch (type)
            {
            case ElementIndexType::UINT16:
                return sizeof(unsigned short);
            case ElementIndexType::UINT32:
                return sizeof(unsigned int);
            case ElementIndexType::UINT8:
                return 1;
            default:
                TUNGSTEN_THROW("Unsupported type: " + std::to_string(int(type)));
            }
        }
    }

    void draw_array(TopologyType topology, int32_t offset, int32_t count)
    {
        get_ogl_wrapper().drawArrays(to_ogl_draw_mode(topology), int32_t(offset), count);
    }

    void draw_elements(TopologyType topology, ElementIndexType type, int32_t offset, int32_t count)
    {
        intptr_t tmp_offset = offset * get_type_size(type);
        get_ogl_wrapper().drawElements(to_ogl_draw_mode(topology),
                                       count,
                                       to_ogl_element_index_type(type),
                                       reinterpret_cast<void*>(tmp_offset));
        THROW_IF_GL_ERROR();
    }

    void draw_elements_16(TopologyType topology, int32_t offset, int32_t count)
    {
        draw_elements(topology, ElementIndexType::UINT16, offset, count);
    }
}
