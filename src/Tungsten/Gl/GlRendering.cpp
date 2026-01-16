//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlRendering.hpp"

#include "Tungsten/Gl/IOglWrapper.hpp"
#include "Tungsten/TungstenException.hpp"
#include "GlTypeConversion.hpp"

namespace Tungsten
{
    void set_clear_color(float r, float g, float b, float a)
    {
        get_ogl_wrapper().clear_color(r, g, b, a);
    }

    void set_clear_color(const Xyz::Vector4F& color)
    {
        set_clear_color(color[0], color[1], color[2], color[3]);
    }

    void set_clear_depth(float depth)
    {
        get_ogl_wrapper().clear_depth(depth);
    }

    void set_clear_stencil(int32_t stencil)
    {
        get_ogl_wrapper().clear_stencil(stencil);
    }

    void clear(ClearBits mask)
    {
        get_ogl_wrapper().clear(static_cast<GLenum>(mask));
    }

    void finish_rendering()
    {
        get_ogl_wrapper().finish();
    }
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
        get_ogl_wrapper().draw_arrays(to_ogl_draw_mode(topology), int32_t(offset), count);
    }

    void draw_elements(TopologyType topology, ElementIndexType type, int32_t offset, int32_t count)
    {
        intptr_t tmp_offset = offset * get_type_size(type);
        get_ogl_wrapper().draw_elements(to_ogl_draw_mode(topology),
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
