//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-08-21.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include <cassert>
#include <string_view>

namespace ParserTools
{
    template <typename FindDelimiterFunc>
    class StringDelimiterIterator
    {
    public:
        StringDelimiterIterator() = default;

        StringDelimiterIterator(std::string_view str, FindDelimiterFunc func)
            : str_(str),
              find_delimiter_func_(std::move(func))
        {
        }

        bool next()
        {
            str_ = str_.substr(delimiter_end_);
            if (str_.empty())
            {
                delimiter_start_ = delimiter_end_ = 0;
                return false;
            }

            auto [s, e] = find_delimiter_func_(str_);
            assert(s <= e);
            assert(e <= str_.size());
            delimiter_start_ = s;
            delimiter_end_ = e;
            return true;
        }

        /**
         * @brief Returns the substring between the previous delimiter
         *  and the current delimiter, or the start of the string and the
         *  current delimiter if there is no previous delimiter.
         */
        [[nodiscard]]
        std::string_view string() const
        {
            return str_.substr(0, delimiter_start_);
        }

        [[nodiscard]]
        std::string_view delimiter() const
        {
            return str_.substr(delimiter_start_,
                               delimiter_end_ - delimiter_start_);
        }

        [[nodiscard]]
        std::string_view remainder() const
        {
            return str_.substr(delimiter_end_);
        }

    private:
        std::string_view str_;
        size_t delimiter_start_ = 0;
        size_t delimiter_end_ = 0;
        FindDelimiterFunc find_delimiter_func_;
    };
}
