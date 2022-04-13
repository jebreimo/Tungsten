//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlContext.hpp"

#include <stdexcept>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    GlContext GlContext::create(SDL_Window* window)
    {
        auto context = GlContext{window};
        if (!context.context_)
            TUNGSTEN_THROW("Unable to create GlContext.");
        return context;
    }

    GlContext::GlContext()
            : context_(nullptr)
    {
    }

    GlContext::GlContext(SDL_Window* window)
            : context_(SDL_GL_CreateContext(window))
    {
    }

    GlContext::GlContext(GlContext&& other)
            : context_(other.context_)
    {
        other.context_ = nullptr;
    }

    GlContext::~GlContext()
    {
        if (context_)
            SDL_GL_DeleteContext(context_);
    }

    GlContext& GlContext::operator=(GlContext&& other)
    {
        if (context_)
            SDL_GL_DeleteContext(context_);
        context_ = other.context_;
        other.context_ = nullptr;
        return *this;
    }

    GlContext::operator SDL_GLContext() const
    {
        return context_;
    }
}
