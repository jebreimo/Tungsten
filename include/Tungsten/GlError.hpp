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

namespace Tungsten
{
    std::string getGlErrorMessage(GLenum errorCode);

    std::string formatErrorMessage(const std::string& error,
                                   const std::string& fileName,
                                   long lineNo,
                                   const std::string& funcName);

    class GlError : public std::runtime_error
    {
    public:
        GlError(GLenum errorCode) noexcept
            : GlError(errorCode, "", -1, "")
        {}

        GlError(GLenum errorCode,
                 const std::string& fileName,
                 long lineNo,
                 const std::string& funcName) noexcept
            : GlError(getGlErrorMessage(errorCode), fileName, -1, funcName)
        {}

        GlError(const std::string& message,
                 const std::string& fileName,
                 long lineNo,
                 const std::string& funcName) noexcept
            : std::runtime_error(formatErrorMessage(message, fileName,
                                                    lineNo, funcName))
        {}
    };

}

#define GL_THROW(msg) \
        throw GlError((msg), __FILE__, __LINE__, __FUNCTION__)

#define THROW_IF_GL_ERROR() \
    do { \
        auto e = glGetError(); \
        if (!e) \
            break; \
        throw GlError(e, __FILE__, __LINE__, __FUNCTION__); \
    } while (false)
