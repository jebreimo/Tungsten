//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Font.hpp"
#include "Types.hpp"

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

        TextRenderer(const TextRenderer&) = delete;
        TextRenderer(TextRenderer&&) noexcept;
        ~TextRenderer();
        TextRenderer& operator=(const TextRenderer&) = delete;
        TextRenderer& operator=(TextRenderer&&) noexcept;

        [[nodiscard]] const Font& font() const;

        [[nodiscard]] bool auto_blend() const;

        void set_auto_blend(bool value);

        void draw(std::u32string_view text,
                  const Xyz::Vector2F& pos,
                  const Xyz::Vector2F& screen_size,
                  const TextProperties& properties = {});
    private:
        void initialize();

        Size2D image_size() const;

        struct Data;
        std::unique_ptr<Data> data_;
        const Font* font_ = nullptr;
        bool auto_blend_ = true;
    };

    [[nodiscard]] Xyz::Vector2F
    get_size(std::u32string_view text, const Font& font,
             float line_gap = 0.1);
}
