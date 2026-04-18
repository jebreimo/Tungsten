//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>

namespace Tungsten
{
    std::u32string utf8_to_utf32(std::string_view text);

    std::u32string utf8_to_utf32(std::u8string_view text);
}
