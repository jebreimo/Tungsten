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
#include <SDL2/SDL.h>

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
    #define IMPL_TUNGSTEN_THROW_2(file, line, msg) \
        throw ::Tungsten::TungstenException(file "(" #line "): " msg)
#else
    #define IMPL_TUNGSTEN_THROW_2(file, line, msg) \
        throw ::Tungsten::TungstenException(file ":" #line ": " msg)
#endif

#define IMPL_TUNGSTEN_THROW_1(file, line, msg) \
    IMPL_TUNGSTEN_THROW_2(file, line, msg)

#define TUNGSTEN_THROW(msg) \
    IMPL_TUNGSTEN_THROW_1(__FILE__, __LINE__, msg)

#define THROW_IF_GL_ERROR() \
    do { \
        auto my_gl_error = glGetError(); \
        if (!my_gl_error) \
            break; \
        IMPL_TUNGSTEN_THROW_1(__FILE__, __LINE__, \
                          + std::string("[") + __func__ + "] " \
                          + get_gl_error_message(my_gl_error)); \
    } while (false)

#define THROW_SDL_ERROR() \
    IMPL_TUNGSTEN_THROW_1(__FILE__, __LINE__, \
                      + std::string("[") + __func__ + "] " + SDL_GetError())
