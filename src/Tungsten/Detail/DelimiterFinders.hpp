//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-08-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <algorithm>
#include <string_view>
#include <cctype>

namespace ParserTools
{
    template <typename CharT = char>
    struct FindSubstring
    {
        FindSubstring() = default;

        explicit FindSubstring(std::basic_string_view<CharT> str)
            : substring_(str)
        {
        }

        std::pair<size_t, size_t> operator()(std::basic_string_view<CharT> str) const
        {
            if (substring_.empty())
                return {str.size(), str.size()};
            auto it = std::search(str.begin(), str.end(),
                                  substring_.begin(), substring_.end());
            auto start = std::distance(str.begin(), it);
            auto end = it != str.end() ? start + substring_.size() : start;
            return {start, end};
        }

    private:
        std::basic_string_view<CharT> substring_;
    };

    template <typename CharT = char>
    struct FindChar
    {
        FindChar() = default;

        explicit FindChar(CharT ch)
            : char_(ch)
        {
        }

        std::pair<size_t, size_t> operator()(std::basic_string_view<CharT> str) const
        {
            const auto it = std::find(str.begin(), str.end(), char_);
            auto start = std::distance(str.begin(), it);
            auto end = it != str.end() ? start + 1 : start;
            return {start, end};
        }

    private:
        CharT char_ = CharT('\0');
    };

    template <typename CharT = char>
    struct FindNewline
    {
        std::pair<size_t, size_t> operator()(std::basic_string_view<CharT> str) const
        {
            auto from = std::find_if(str.begin(), str.end(),
                                     [](CharT c) { return c == CharT('\n') || c == CharT('\r'); });
            auto to = from;
            if (to != str.end() && *to++ == CharT('\r')
                && to != str.end() && *to == CharT('\n'))
            {
                ++to;
            }
            return {
                std::distance(str.begin(), from),
                std::distance(str.begin(), to)
            };
        }
    };

    template <typename CharT = char>
    struct FindWhitespace
    {
        std::pair<size_t, size_t> operator()(std::basic_string_view<CharT> str) const
        {
            auto from = std::find_if(str.begin(), str.end(), [](CharT c)
            {
                return std::isspace(static_cast<unsigned char>(c)) != 0;
            });

            auto to = std::find_if(from, str.end(), [](CharT c)
            {
                return std::isspace(static_cast<unsigned char>(c)) == 0;
            });

            return {
                std::distance(str.begin(), from),
                std::distance(str.begin(), to)
            };
        }
    };

    template <typename CharT = char>
    struct FindSequenceOf
    {
        FindSequenceOf() = default;

        explicit FindSequenceOf(std::basic_string_view<CharT> characters)
            : characters_(characters)
        {
        }

        std::pair<size_t, size_t> operator()(std::basic_string_view<CharT> str) const
        {
            auto match = [this](CharT c)
            {
                return std::find(characters_.begin(),
                                 characters_.end(),
                                 c) != characters_.end();
            };
            auto from = std::find_if(str.begin(), str.end(), match);
            auto to = std::find_if_not(from, str.end(), match);
            return {
                std::distance(str.begin(), from),
                std::distance(str.begin(), to)
            };
        }

    private:
        std::basic_string_view<CharT> characters_;
    };
}
