//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-05-14.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Font.hpp"
#include "Tungsten/FontManager.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Font: get_text_rec exact size")
{
    Tungsten::FontManager font_manager;
    auto& font = font_manager.default_font();
    auto rect = Tungsten::get_text_rect(*font, std::string_view("Hello_World!"), 0.5f, false);
    REQUIRE(rect.origin == Xyz::Vector2F(1, -2));
    REQUIRE(rect.size == Xyz::Vector2F{220, 27});
}

TEST_CASE("Font: get_text_rec with full line height")
{
    Tungsten::FontManager font_manager;
    auto& font = font_manager.default_font();
    auto rect = Tungsten::get_text_rect(*font, std::string_view("Hello_World!"), 0.5f, true);
    REQUIRE(rect.origin == Xyz::Vector2F(1, -7));
    REQUIRE(rect.size == Xyz::Vector2F{220, 33});
}

TEST_CASE("Font: get_text_rec with multi-line string")
{
    Tungsten::FontManager font_manager;
    auto& font = font_manager.default_font();
    auto rect = Tungsten::get_text_rect(*font, std::string_view("Hello\nWorld!"), 0.5f, true);
    REQUIRE(rect.origin == Xyz::Vector2F(0, -57));
    REQUIRE(rect.size == Xyz::Vector2F{107, 83});
}
