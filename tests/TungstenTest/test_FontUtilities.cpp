//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../src/Tungsten/FontUtilities.hpp"
#include "Tungsten/FontManager.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("FontUtilities: add_vertexes with left anchor")
{
    std::u32string text = U"Hello!";
    std::vector<Tungsten::GlyphVertex> vertexes;
    std::vector<int32_t> indexes;
    Tungsten::FontManager font_manager;
    auto& font = font_manager.default_font();
    auto rect = Tungsten::add_vertexes(vertexes, indexes, *font, text, 0.5f, Tungsten::HorizontalAlignment::LEFT);
    REQUIRE(vertexes.size() == text.size() * 4);
    REQUIRE(indexes.size() == text.size() * 6);
    REQUIRE(rect.origin == Xyz::Vector2F(1, -7));
    REQUIRE(rect.size == Xyz::Vector2F(106, font->max_glyph.size.y()));
}

TEST_CASE("FontUtilities: add_vertexes with center anchor")
{
    std::u32string text = U"Hello!";
    std::vector<Tungsten::GlyphVertex> vertexes;
    std::vector<int32_t> indexes;
    Tungsten::FontManager font_manager;
    auto& font = font_manager.default_font();
    auto rect = Tungsten::add_vertexes(vertexes, indexes, *font, text, 0.5f, Tungsten::HorizontalAlignment::CENTER);
    REQUIRE(vertexes.size() == text.size() * 4);
    REQUIRE(indexes.size() == text.size() * 6);
    REQUIRE(rect.origin == Xyz::Vector2F(-52, -7));
    REQUIRE(rect.size == Xyz::Vector2F(106, font->max_glyph.size.y()));
}

TEST_CASE("FontUtilities: add_vertexes with right anchor")
{
    std::u32string text = U"Hello!";
    std::vector<Tungsten::GlyphVertex> vertexes;
    std::vector<int32_t> indexes;
    Tungsten::FontManager font_manager;
    auto& font = font_manager.default_font();
    auto rect = Tungsten::add_vertexes(vertexes, indexes, *font, text, 0.5f, Tungsten::HorizontalAlignment::RIGHT);
    REQUIRE(vertexes.size() == text.size() * 4);
    REQUIRE(indexes.size() == text.size() * 6);
    REQUIRE(rect.origin == Xyz::Vector2F(-105, -7));
    REQUIRE(rect.size == Xyz::Vector2F(106, font->max_glyph.size.y()));
}
