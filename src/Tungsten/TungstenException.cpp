//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <string>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    std::string get_gl_error_message(GLenum error_code)
    {
        switch (error_code)
        {
        case GL_NO_ERROR: return "No error.";
        case GL_INVALID_ENUM: return "Invalid enum.";
        case GL_INVALID_VALUE: return "Invalid value.";
        case GL_INVALID_OPERATION: return "Invalid operation.";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation.";
        case GL_OUT_OF_MEMORY: return "Out of memory.";
        case GL_STACK_UNDERFLOW: return "Stack underflow.";
        case GL_STACK_OVERFLOW: return "Stack overflow.";
        default: return "Unknown error.";
        }
    }
}
