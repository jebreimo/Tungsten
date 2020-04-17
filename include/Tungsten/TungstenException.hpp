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
    std::string getGlErrorMessage(GLenum errorCode);

    std::string formatErrorMessage(const std::string& error,
                                   const std::string& fileName,
                                   long lineNo,
                                   const std::string& funcName);

    class TungstenException : public std::runtime_error
    {
    public:
        TungstenException(const std::string& message,
                const std::string& fileName,
                long lineNo,
                const std::string& funcName) noexcept
            : std::runtime_error(formatErrorMessage(message, fileName,
                                                    lineNo, funcName))
        {}
    };
}

#define TUNGSTEN_THROW(msg) \
        throw TungstenException((msg), __FILE__, __LINE__, __FUNCTION__)

#define THROW_IF_GL_ERROR() \
    do { \
        auto e = glGetError(); \
        if (!e) \
            break; \
        throw TungstenException(getGlErrorMessage(e), __FILE__, __LINE__, __FUNCTION__); \
    } while (false)

#define THROW_SDL_ERROR() \
    throw TungstenException(SDL_GetError(), __FILE__, __LINE__, __FUNCTION__)
