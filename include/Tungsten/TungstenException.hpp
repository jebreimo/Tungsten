//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <stdexcept>

namespace Tungsten
{
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
