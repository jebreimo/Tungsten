//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-27.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/IOglWrapper.h"

#include "OglWrapper.h"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        std::unique_ptr<IOglWrapper> wrapper_;
    }

    IOglWrapper* getOglWrapper()
    {
        if (!wrapper_)
            wrapper_ = std::make_unique<OglWrapper>();
        return wrapper_.get();
    }

    std::unique_ptr<IOglWrapper> setOglWrapper(std::unique_ptr<IOglWrapper> wrapper)
    {
        auto oldWrapper = std::move(wrapper_);
        wrapper_ = std::move(wrapper);
        return oldWrapper;
    }

    std::unique_ptr<IOglWrapper> setOglWrapper(StandardOglWrapper wrapperType)
    {
        std::unique_ptr<IOglWrapper> wrapper = std::make_unique<OglWrapper>();
        switch (wrapperType)
        {
        case StandardOglWrapper::DEFAULT:
            wrapper = std::make_unique<OglWrapper>();
            break;
        // case OpenGlWrappers::TRACING:
        //     wrapper = std::make_unique<TracingOpenGlWrapper>();
        //     break;
        default:
            TUNGSTEN_THROW("Invalid OpenGL wrapper type!");
        }
        wrapper_.swap(wrapper);
        return wrapper;
    }
}
