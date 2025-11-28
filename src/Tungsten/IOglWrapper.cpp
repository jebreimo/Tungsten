//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-27.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/IOglWrapper.hpp"

#include "OglWrapper.hpp"
#include "TracingOglWrapper.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        OglWrapper ogl_wrapper_;
        TracingOglWrapper tracing_ogl_wrapper_;
        IOglWrapper* raw_wrapper_ = &ogl_wrapper_;
        std::unique_ptr<IOglWrapper> custom_wrapper_;

        void set_raw_wrapper(IOglWrapper* wrapper)
        {
            if (raw_wrapper_ == &tracing_ogl_wrapper_)
                tracing_ogl_wrapper_.wrapper = wrapper;
            else
                raw_wrapper_ = wrapper;
        }
    }

    IOglWrapper& get_ogl_wrapper()
    {
        return *raw_wrapper_;
    }

    bool is_ogl_tracing_enabled()
    {
        return raw_wrapper_ == &tracing_ogl_wrapper_;
    }

    void set_ogl_tracing_enabled(bool enabled)
    {
        if (enabled && !is_ogl_tracing_enabled())
        {
            tracing_ogl_wrapper_.wrapper = raw_wrapper_;
            raw_wrapper_ = &tracing_ogl_wrapper_;
        }
        else if (!enabled && is_ogl_tracing_enabled())
        {
            raw_wrapper_ = tracing_ogl_wrapper_.wrapper;
            tracing_ogl_wrapper_.wrapper = nullptr;
        }
    }

    std::unique_ptr<IOglWrapper> set_custom_ogl_wrapper(std::unique_ptr<IOglWrapper> wrapper)
    {
        auto old_wrapper = std::move(custom_wrapper_);
        custom_wrapper_ = std::move(wrapper);
        set_raw_wrapper(custom_wrapper_ ? custom_wrapper_.get() : &ogl_wrapper_);
        return old_wrapper;
    }

    std::unique_ptr<IOglWrapper> set_standard_ogl_wrapper(StandardOglWrapper wrapper_type)
    {
        std::unique_ptr<IOglWrapper> old_wrapper;
        switch (wrapper_type)
        {
            case StandardOglWrapper::DEFAULT:
            set_raw_wrapper(&ogl_wrapper_);
            return std::move(custom_wrapper_);
        default:
            TUNGSTEN_THROW("Invalid OpenGL wrapper type!");
        }
    }
}
