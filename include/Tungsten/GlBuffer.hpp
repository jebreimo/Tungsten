//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct BufferDeleter
    {
        void operator()(GLuint id) const;
    };

    using BufferHandle = GlHandle<BufferDeleter>;

    BufferHandle generate_buffer();

    void generate_buffers(std::span<BufferHandle> buffers);

    void bind_buffer(GLenum target, GLuint buffer);

    void set_buffer_data(GLenum target, GLsizeiptr size,
                         const GLvoid* data, GLenum usage);

    void set_buffer_subdata(GLenum target, GLintptr offset,
                            GLsizeiptr size, const GLvoid* data);

    void set_element_array_buffer(GLuint buffer_id,
                                  GLsizeiptr value_count,
                                  const uint16_t* values,
                                  GLenum usage);

    [[nodiscard]]
    bool is_buffer(GLuint buffer);

    [[nodiscard]]
    GLsizei get_buffer_size(GLenum target);

    [[nodiscard]]
    GLenum get_buffer_usage(GLenum target);
}
