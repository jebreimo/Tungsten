//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/TungstenException.hpp>
#include <exception>
#include <string>
#include <SDL3/SDL_error.h>
#include "../../include/Tungsten/Gl/IOglWrapper.hpp"

namespace Tungsten
{
    std::string get_gl_error_message(GLenum error_code);
}

#define THROW_GL_ERROR(error_code) \
    do { \
        throw IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, \
                          + std::string("[") + __func__ + "] " \
                          + get_gl_error_message(error_code)); \
    } while (false)

#define THROW_IF_GL_ERROR() \
    do { \
        auto my_gl_error = get_ogl_wrapper().get_error(); \
        if (!my_gl_error) \
            break; \
        throw IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, \
                          + std::string("[") + __func__ + "] " \
                          + get_gl_error_message(my_gl_error)); \
    } while (false)

#define THROW_SDL_ERROR() \
    throw IMPL_TUNGSTEN_EXCEPTION_1(__FILE__, __LINE__, \
                      + std::string("[") + __func__ + "] " + SDL_GetError())
