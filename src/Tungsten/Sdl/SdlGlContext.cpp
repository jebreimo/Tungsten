//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Sdl/SdlGlContext.hpp"

#include "../ExceptionHelpers.hpp"

namespace Tungsten
{
    SdlGlContext SdlGlContext::create(SDL_Window* window)
    {
        try
        {
            return SdlGlContext{window};
        }
        catch (...)
        {
            TUNGSTEN_THROW_NESTED("Unable to create OpenGL context.");
        }
    }

    SdlGlContext::SdlGlContext()
            : context_(nullptr)
    {
    }

    SdlGlContext::SdlGlContext(SDL_Window* window)
            : context_(SDL_GL_CreateContext(window))
    {
        if (!context_)
            THROW_SDL_ERROR();
    }

    SdlGlContext::SdlGlContext(SdlGlContext&& other) noexcept
        : context_(other.context_)
    {
        other.context_ = nullptr;
    }

    SdlGlContext::~SdlGlContext()
    {
        if (context_)
            SDL_GL_DestroyContext(context_);
    }

    SdlGlContext& SdlGlContext::operator=(SdlGlContext&& other) noexcept
    {
        if (context_)
            SDL_GL_DestroyContext(context_);
        context_ = other.context_;
        other.context_ = nullptr;
        return *this;
    }

    SdlGlContext::operator SDL_GLContext() const
    {
        return context_;
    }
}
