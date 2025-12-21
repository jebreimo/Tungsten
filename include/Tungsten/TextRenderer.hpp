//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Font.hpp"
#include "GlTypes.hpp"

namespace Tungsten
{
    struct TextProperties
    {
        Yimage::Rgba8 color = Yimage::Rgba8(0xFFFFFFFF);
        float line_gap = 0.1;
    };

    class TextRenderer
    {
    public:
        explicit TextRenderer(const Font& font);

        ~TextRenderer();

        TextRenderer(const TextRenderer&) = delete;
        TextRenderer& operator=(const TextRenderer&) = delete;

        TextRenderer(TextRenderer&&) noexcept;
        TextRenderer& operator=(TextRenderer&&) noexcept;

        [[nodiscard]] const Font& font() const;

        [[nodiscard]] bool auto_blend() const;

        void set_auto_blend(bool value);

        [[nodiscard]] Xyz::Vector2F
        get_size(std::string_view text, float line_gap = 0.1) const;

        [[nodiscard]] Xyz::Vector2F
        get_size(std::u8string_view text, float line_gap = 0.1) const;

        [[nodiscard]] Xyz::Vector2F
        get_size(std::u32string_view text, float line_gap = 0.1) const;

        void draw(std::string_view text,
                  const Xyz::Vector2F& pos,
                  const Xyz::Vector2F& screen_size,
                  const TextProperties& properties = {}) const;

        void draw(std::u8string_view text,
                  const Xyz::Vector2F& pos,
                  const Xyz::Vector2F& screen_size,
                  const TextProperties& properties = {}) const;

        void draw(std::u32string_view text,
                  const Xyz::Vector2F& pos,
                  const Xyz::Vector2F& screen_size,
                  const TextProperties& properties = {}) const;
    private:
        [[nodiscard]] Size2I image_size() const;

        template <typename CharT>
        [[nodiscard]] std::u32string
        to_u32(std::basic_string_view<CharT> str) const;

        struct Data;
        std::unique_ptr<Data> data_;
    };
}
