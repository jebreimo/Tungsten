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
#include <concepts>
#include <type_traits>
#include <utility>

namespace ParserTools
{
    template <typename F, typename CharT>
    concept FindDelimiterFunc =
        std::invocable<F, std::basic_string_view<CharT>> &&
        std::same_as<std::invoke_result_t<F, std::basic_string_view<CharT>>,
                     std::pair<std::size_t, std::size_t>>;

    template <typename FindDelimiterFuncT, typename CharT = char>
        requires FindDelimiterFunc<FindDelimiterFuncT, CharT>
    class StringDelimiterIterator
    {
    public:
        StringDelimiterIterator() = default;

        StringDelimiterIterator(std::basic_string_view<CharT> str, FindDelimiterFuncT func)
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
        std::basic_string_view<CharT> string() const
        {
            return str_.substr(0, delimiter_start_);
        }

        [[nodiscard]]
        std::basic_string_view<CharT> delimiter() const
        {
            return str_.substr(delimiter_start_,
                               delimiter_end_ - delimiter_start_);
        }

        [[nodiscard]]
        std::basic_string_view<CharT> remainder() const
        {
            return str_.substr(delimiter_end_);
        }

    private:
        std::basic_string_view<CharT> str_;
        size_t delimiter_start_ = 0;
        size_t delimiter_end_ = 0;
        FindDelimiterFuncT find_delimiter_func_;
    };
}
