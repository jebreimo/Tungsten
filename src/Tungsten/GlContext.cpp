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
        if (!context.m_context)
            TUNGSTEN_THROW("Unable to create GlContext.");
        return context;
    }

    GlContext::GlContext()
            : m_context(nullptr)
    {
    }

    GlContext::GlContext(SDL_Window* window)
            : m_context(SDL_GL_CreateContext(window))
    {
    }

    GlContext::GlContext(GlContext&& other)
            : m_context(other.m_context)
    {
        other.m_context = nullptr;
    }

    GlContext::~GlContext()
    {
        if (m_context)
            SDL_GL_DeleteContext(m_context);
    }

    GlContext& GlContext::operator=(GlContext&& other)
    {
        if (m_context)
            SDL_GL_DeleteContext(m_context);
        m_context = other.m_context;
        other.m_context = nullptr;
        return *this;
    }

    GlContext::operator SDL_GLContext() const
    {
        return m_context;
    }
}
