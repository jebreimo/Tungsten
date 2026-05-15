//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-05-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../src/Tungsten/TextUtilities.hpp"

#include <catch2/catch_test_macros.hpp>

template <typename CharT>
using StringView = std::basic_string_view<CharT>;

TEST_CASE("TextUtilities: utf8_to_utf32")
{
    REQUIRE(Tungsten::utf8_to_utf32(u8"Hello, world!") == U"Hello, world!");
    REQUIRE(Tungsten::utf8_to_utf32(u8"こんにちは") == U"こんにちは");
    REQUIRE(Tungsten::utf8_to_utf32(u8"😀") == U"😀");
}

TEST_CASE("TextUtilities: split_lines")
{
    auto lines = Tungsten::split_lines(StringView(u8"Hello\nWorld"));
    REQUIRE(lines.size() == 2);
    REQUIRE(lines[0] == u8"Hello");
    REQUIRE(lines[1] == u8"World");
}

TEST_CASE("TextUtilities: split_lines with empty string")
{
    const auto lines = Tungsten::split_lines(StringView(u8""));
    REQUIRE(lines.empty());
}

TEST_CASE("TextUtilities: split_lines with just a newline")
{
    const auto lines = Tungsten::split_lines(StringView(u8"\n"));
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0].empty());
}

TEST_CASE("TextUtilities: split_lines with single line")
{
    auto lines = Tungsten::split_lines(StringView(u8"Hello"));
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0] == u8"Hello");
}
