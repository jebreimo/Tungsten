//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-11-20.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlBuffer.hpp"

namespace Tungsten
{
    template <typename T>
    class Buffer
    {
    public:
        explicit Buffer(GLenum target, GLenum usage = GL_STATIC_DRAW)
            : buffer_(generate_buffer()),
              target_(target),
              usage_(usage)
        {}

        Buffer(std::span <T> data, GLenum target,
                 GLenum usage = GL_STATIC_DRAW)
            : buffer_(generate_buffer()),
              target_(target),
              usage_(usage)
        {
            set_data(data);
        }

        void bind() const
        {
            bind_buffer(target_, buffer_);
        }

        void reserve(size_t capacity)
        {
            if (capacity_ < capacity)
            {
                bind();
                set_buffer_data(target_, capacity * sizeof(T), nullptr,
                                usage_);
                capacity_ = capacity;
            }
        }

        [[nodiscard]]
        size_t size() const
        {
            return size_;
        }

        void resize(size_t size)
        {
            reserve(size);
            size_ = size;
        }

        void set_data(std::span<const T> data)
        {
            bind();
            if (capacity_ < data.size())
            {
                set_buffer_data(target_, data.size() * sizeof(T), data.data(),
                                usage_);
                capacity_ = size_ = data.size();
            }
            else
            {
                set_buffer_subdata(target_, 0, data.size() * sizeof(T),
                                   data.data());
                size_ = data.size();
            }
        }

        void set_data(std::span<const T> data, GLsizei offset)
        {
            if (offset == 0)
            {
                set_data(data);
                return;
            }

            bind();
            if (data.size() + offset > capacity_)
            {
                set_buffer_data(target_, (data.size() + offset) * sizeof(T),
                                nullptr, usage_);
                capacity_ = data.size() + offset;
            }
            set_buffer_subdata(target_, offset * sizeof(T),
                               data.size() * sizeof(T), data.data());
            size_ = std::max(size_, offset + data.size());
        }

    private:
        BufferHandle buffer_;
        GLenum target_;
        GLenum usage_;
        size_t size_ = 0;
        size_t capacity_ = 0;
    };
}
