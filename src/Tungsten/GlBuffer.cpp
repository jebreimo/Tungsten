//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlBuffer.hpp"

#include <vector>

#include "GlTypeConversion.h"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void BufferDeleter::operator()(uint32_t id) const
    {
        glDeleteBuffers(1, &id);
        THROW_IF_GL_ERROR();
    }

    BufferHandle generate_buffer()
    {
        uint32_t id;
        glGenBuffers(1, &id);
        THROW_IF_GL_ERROR();
        return BufferHandle(id);
    }

    void generate_buffers(std::span<BufferHandle> buffers)
    {
        auto ids = std::vector<uint32_t>(size_t(buffers.size()));
        glGenBuffers(int32_t(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            buffers[i] = BufferHandle(ids[i]);
    }

    void bind_buffer(BufferTarget target, uint32_t buffer)
    {
        glBindBuffer(to_ogl_buffer_target(target), buffer);
        THROW_IF_GL_ERROR();
    }

    void set_buffer_data(BufferTarget target, ptrdiff_t size, const void* data,
                         BufferUsage usage)
    {
        glBufferData(to_ogl_buffer_target(target), size, data, to_ogl_buffer_usage(usage));
        THROW_IF_GL_ERROR();
    }

    void set_buffer_subdata(BufferTarget target, ptrdiff_t offset,
                            ptrdiff_t size, const void* data)
    {
        glBufferSubData(to_ogl_buffer_target(target), offset, size, data);
        THROW_IF_GL_ERROR();
    }

    void set_element_array_buffer(uint32_t buffer_id,
                                  ptrdiff_t value_count,
                                  const uint16_t* values,
                                  BufferUsage usage)
    {
        bind_buffer(BufferTarget::ELEMENT_ARRAY, buffer_id);
        set_buffer_data(BufferTarget::ELEMENT_ARRAY,
                        ptrdiff_t(value_count * sizeof(uint16_t)),
                        values,
                        usage);
    }

    bool is_buffer(uint32_t buffer)
    {
        return glIsBuffer(buffer) != 0;
    }

    int32_t get_buffer_size(BufferTarget target)
    {
        int32_t size;
        glGetBufferParameteriv(to_ogl_buffer_target(target), GL_BUFFER_SIZE, &size);
        THROW_IF_GL_ERROR();
        return size;
    }

    BufferUsage get_buffer_usage(BufferTarget target)
    {
        int32_t usage;
        glGetBufferParameteriv(to_ogl_buffer_target(target), GL_BUFFER_USAGE, &usage);
        THROW_IF_GL_ERROR();
        return from_ogl_buffer_usage(usage);
    }
}
