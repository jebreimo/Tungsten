//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace Tungsten
{
    std::u32string utf8_to_utf32(std::string_view text);

    std::u32string utf8_to_utf32(std::u8string_view text);

    template <typename T>
    auto split_lines(std::basic_string_view<T> text)
        -> std::vector<std::basic_string_view<T>>
    {
        std::vector<std::basic_string_view<T>> lines;
        size_t start = 0;
        while (start < text.size())
        {
            auto newline_pos = text.find(U'\n', start);
            if (newline_pos == std::basic_string_view<T>::npos)
                newline_pos = text.size();
            lines.emplace_back(text.substr(start, newline_pos - start));
            start = newline_pos + 1;
        }
        return lines;
    }
}
