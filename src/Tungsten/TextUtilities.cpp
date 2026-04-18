//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "TextUtilities.hpp"

#include <Yconvert/Convert.hpp>

namespace Tungsten
{
    std::u32string utf8_to_utf32(std::string_view text)
    {
        return Yconvert::convert_to<std::u32string>(
            text,
            Yconvert::Encoding::UTF_8,
            Yconvert::Encoding::UTF_32_NATIVE,
            Yconvert::ErrorPolicy::THROW
        );
    }

    std::u32string utf8_to_utf32(std::u8string_view text)
    {
        return Yconvert::convert_to<std::u32string>(
            text,
            Yconvert::Encoding::UTF_8,
            Yconvert::Encoding::UTF_32_NATIVE,
            Yconvert::ErrorPolicy::THROW
        );
    }
}
