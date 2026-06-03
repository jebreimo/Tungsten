//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlBuffer.hpp"

#include <vector>

#include "GlTypeConversion.hpp"
#include "Tungsten/Gl/IOglWrapper.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"

namespace Tungsten
{
    void BufferDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().delete_buffers(1, &id);
        THROW_IF_GL_ERROR();
    }

    BufferHandle generate_buffer()
    {
        uint32_t id;
        get_ogl_wrapper().gen_buffers(1, &id);
        THROW_IF_GL_ERROR();
        return BufferHandle(id);
    }

    BufferHandle generate_buffer(BufferTarget target)
    {
        auto buffer = generate_buffer();
        bind_buffer(target, buffer.id());
        return buffer;
    }

    BufferHandle generate_buffer(BufferTarget target, ptrdiff_t size, BufferUsage usage)
    {
        auto buffer = generate_buffer(target);
        allocate_buffer(target, size, usage);
        return buffer;
    }

    void generate_buffers(std::span<BufferHandle> buffers)
    {
        auto ids = std::vector<uint32_t>(size_t(buffers.size()));
        get_ogl_wrapper().gen_buffers(int32_t(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            buffers[i] = BufferHandle(ids[i]);
    }

    void generate_buffers(std::span<BufferHandle*> buffers)
    {
        auto ids = std::vector<uint32_t>(size_t(buffers.size()));
        get_ogl_wrapper().gen_buffers(int32_t(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            *buffers[i] = BufferHandle(ids[i]);
    }

    void bind_buffer(BufferTarget target, uint32_t buffer)
    {
        get_ogl_wrapper().bind_buffer(to_ogl_buffer_target(target), buffer);
        THROW_IF_GL_ERROR();
    }

    void set_buffer_data(BufferTarget target, ptrdiff_t size, const void* data,
                         BufferUsage usage)
    {
        get_ogl_wrapper().buffer_data(to_ogl_buffer_target(target), size, data,
                                      to_ogl_buffer_usage(usage));
        THROW_IF_GL_ERROR();
    }

    void set_buffer_subdata(BufferTarget target, ptrdiff_t offset,
                            ptrdiff_t size, const void* data)
    {
        get_ogl_wrapper().buffer_sub_data(to_ogl_buffer_target(target), offset, size, data);
        THROW_IF_GL_ERROR();
    }

    void copy_buffer(BufferTarget read_target,
                     ptrdiff_t read_offset,
                     BufferTarget write_target,
                     ptrdiff_t write_offset,
                     ptrdiff_t size)
    {
        get_ogl_wrapper().copy_buffer_sub_data(to_ogl_buffer_target(read_target),
                                               to_ogl_buffer_target(write_target),
                                               read_offset, write_offset, size);
        THROW_IF_GL_ERROR();
    }

    bool is_buffer(uint32_t buffer)
    {
        return get_ogl_wrapper().is_buffer(buffer) != 0;
    }

    ptrdiff_t get_buffer_size(BufferTarget target)
    {
        int64_t size;
        get_ogl_wrapper().get_buffer_parameter64(to_ogl_buffer_target(target), GL_BUFFER_SIZE,
                                                 &size);
        THROW_IF_GL_ERROR();
        return size;
    }

    BufferUsage get_buffer_usage(BufferTarget target)
    {
        int32_t usage;
        get_ogl_wrapper().get_buffer_parameter(to_ogl_buffer_target(target), GL_BUFFER_USAGE,
                                               &usage);
        THROW_IF_GL_ERROR();
        return from_ogl_buffer_usage(usage);
    }

    namespace
    {
        GLenum to_ogl_buffer_binding(BufferTarget target)
        {
            switch (target)
            {
            case BufferTarget::ARRAY:
                return GL_ARRAY_BUFFER_BINDING;
            case BufferTarget::ELEMENT_ARRAY:
                return GL_ELEMENT_ARRAY_BUFFER_BINDING;
            case BufferTarget::COPY_READ:
                return GL_COPY_READ_BUFFER_BINDING;
            case BufferTarget::COPY_WRITE:
                return GL_COPY_WRITE_BUFFER_BINDING;
            case BufferTarget::PIXEL_PACK:
                return GL_PIXEL_PACK_BUFFER_BINDING;
            case BufferTarget::PIXEL_UNPACK:
                return GL_PIXEL_UNPACK_BUFFER_BINDING;
            case BufferTarget::TRANSFORM_FEEDBACK:
                return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
            case BufferTarget::UNIFORM:
                return GL_UNIFORM_BUFFER_BINDING;
            default:
                TUNGSTEN_THROW("Invalid buffer target!");
            }
        }
    }

    uint32_t get_bound_buffer(BufferTarget target)
    {
        return get_int32_value(to_ogl_buffer_binding(target));
    }

    BufferRestorer::BufferRestorer(BufferTarget target)
        : target_(target),
          previous_buffer_id_(get_bound_buffer(target))
    {
    }

    BufferRestorer::BufferRestorer(BufferTarget target,
                                             uint32_t previous_buffer_id)
        : target_(target),
          previous_buffer_id_(previous_buffer_id)
    {
    }

    BufferRestorer::~BufferRestorer()
    {
        bind_buffer(target_, previous_buffer_id_);
    }
}
