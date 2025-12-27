//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 19.04.2017.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>

namespace Tungsten
{
    template <typename Deleter>
    class GlHandle
    {
    public:
        GlHandle()
            : id_()
        {}

        explicit GlHandle(uint32_t id)
            : id_(id)
        {}

        GlHandle(const GlHandle&) = delete;

        GlHandle(GlHandle&& other) noexcept
            : id_(other.id_)
        {
            other.id_ = 0;
        }

        ~GlHandle()
        {
            reset();
        }

        GlHandle& operator=(const GlHandle&) = delete;

        GlHandle& operator=(GlHandle&& other) noexcept
        {
            reset(other.id_);
            other.id_ = 0;
            return *this;
        }

        operator uint32_t() const
        {
            return id_;
        }

        void reset(uint32_t id = 0)
        {
            if (id_)
                Deleter()(id_);
            id_ = id;
        }

        uint32_t release()
        {
            auto id = id_;
            id_ = 0;
            return id;
        }
    private:
        uint32_t id_;
    };
}
