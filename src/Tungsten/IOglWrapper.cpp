//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-27.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/IOglWrapper.hpp"

#include "OglWrapper.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        OglWrapper ogl_wrapper;
        IOglWrapper* raw_wrapper_ = &ogl_wrapper;
        std::unique_ptr<IOglWrapper> custom_wrapper_;
    }

    IOglWrapper& getOglWrapper()
    {
        return *raw_wrapper_;
    }

    std::unique_ptr<IOglWrapper> setCustomOglWrapper(std::unique_ptr<IOglWrapper> wrapper)
    {
        auto oldWrapper = std::move(custom_wrapper_);
        custom_wrapper_ = std::move(wrapper);
        if (!custom_wrapper_)
            raw_wrapper_ = &ogl_wrapper;
        return oldWrapper;
    }

    std::unique_ptr<IOglWrapper> setOglWrapper(StandardOglWrapper wrapperType)
    {
        std::unique_ptr<IOglWrapper> oldWrapper;
        switch (wrapperType)
        {
            case StandardOglWrapper::DEFAULT:
            raw_wrapper_ = &ogl_wrapper;
            return std::move(custom_wrapper_);
        // case OpenGlWrappers::TRACING:
        //     wrapper = std::make_unique<TracingOpenGlWrapper>();
        //     break;
        default:
            TUNGSTEN_THROW("Invalid OpenGL wrapper type!");
        }
    }
}
