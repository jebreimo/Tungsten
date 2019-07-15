//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <sstream>
#include <string>
#include "Tungsten/GlError.hpp"

namespace Tungsten
{
    std::string getGlErrorMessage(GLenum errorCode)
    {
        switch (errorCode)
        {
        case GL_NO_ERROR: return "No error.";
        case GL_INVALID_ENUM: return "Invalid enum.";
        case GL_INVALID_VALUE: return "Invalid value.";
        case GL_INVALID_OPERATION: return "Invalid operation.";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation.";
        case GL_OUT_OF_MEMORY: return "Out of memory.";
        case GL_STACK_UNDERFLOW: return "Stack underflow.";
        case GL_STACK_OVERFLOW: return "Stack overflow.";
        default: break;
        }
        return "Unknown error.";
    }

    std::string formatErrorMessage(const std::string& message,
                                   const std::string& fileName,
                                   long lineNo,
                                   const std::string& funcName)
    {
        std::stringstream ss;
        if (!funcName.empty())
            ss << funcName << "() in ";
        ss << fileName << ":" << lineNo << ": " << message;
        return ss.str();
    }
}
