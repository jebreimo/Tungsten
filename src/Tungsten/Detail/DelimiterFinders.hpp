//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-08-22.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <algorithm>
#include <string_view>
#include <cctype>

namespace ParserTools
{
    struct FindSubstring
    {
        FindSubstring() = default;

        explicit FindSubstring(std::string_view str)
            : substring_(str)
        {
        }

        std::pair<size_t, size_t> operator()(std::string_view str) const
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
        std::string_view substring_;
    };

    struct FindChar
    {
        FindChar() = default;

        explicit FindChar(char ch)
            : char_(ch)
        {
        }

        std::pair<size_t, size_t> operator()(std::string_view str) const
        {
            const auto it = std::find(str.begin(), str.end(), char_);
            auto start = std::distance(str.begin(), it);
            auto end = it != str.end() ? start + 1 : start;
            return {start, end};
        }

    private:
        char char_ = '\0';
    };

    struct FindNewline
    {
        std::pair<size_t, size_t> operator()(std::string_view str) const
        {
            auto from = std::find_if(str.begin(), str.end(),
                                     [](char c) { return c == '\n' || c == '\r'; });
            auto to = from;
            if (to != str.end() && *to++ == '\r'
                && to != str.end() && *to == '\n')
            {
                ++to;
            }
            return {
                std::distance(str.begin(), from),
                std::distance(str.begin(), to)
            };
        }
    };

    struct FindWhitespace
    {
        std::pair<size_t, size_t> operator()(std::string_view str) const
        {
            auto from = std::find_if(str.begin(), str.end(), [](char c)
            {
                return isspace(c) != 0;
            });

            auto to = std::find_if(from, str.end(), [](char c)
            {
                return isspace(c) == 0;
            });

            return {
                std::distance(str.begin(), from),
                std::distance(str.begin(), to)
            };
        }
    };

    struct FindSequenceOf
    {
        FindSequenceOf() = default;

        explicit FindSequenceOf(std::string_view characters)
            : characters_(characters)
        {
        }

        std::pair<size_t, size_t> operator()(std::string_view str) const
        {
            auto match = [this](char c)
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
        std::string_view characters_;
    };
}
