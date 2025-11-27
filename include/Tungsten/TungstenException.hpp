//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <stdexcept>
#include <string>
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include "IOglWrapper.hpp"

namespace Tungsten
{
    std::string get_gl_error_message(GLenum error_code);

    class TungstenException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
}

#ifdef _MSC_VER
    #define IMPL_TUNGSTEN_EXCEPTION_2(file, line, msg) \
        ::Tungsten::TungstenException(file "(" #line "): " msg)
#else
#define IMPL_TUNGSTEN_EXCEPTION_2(file, line, msg) \
        ::Tungsten::TungstenException(file ":" #line ": " msg)
#endif

#define IMPL_TUNGSTEN_EXCEPTION_1(file, line, msg) \
    IMPL_TUNGSTEN_EXCEPTION_2(file, line, msg)

#define TUNGSTEN_THROW(msg) \
    throw IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, msg)

#define TUNGSTEN_THROW_NESTED(msg) \
    std::throw_with_nested(IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, msg))

#define THROW_IF_GL_ERROR() \
    do { \
        auto my_gl_error = getOglWrapper()->getError(); \
        if (!my_gl_error) \
            break; \
        throw IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, \
                          + std::string("[") + __func__ + "] " \
                          + get_gl_error_message(my_gl_error)); \
    } while (false)

#define THROW_SDL_ERROR() \
    throw IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, \
                      + std::string("[") + __func__ + "] " + SDL_GetError())
