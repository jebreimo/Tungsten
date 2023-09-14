//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <map>
#include "Font.hpp"

namespace Tungsten
{
    class FontManager
    {
    public:
        static FontManager& instance();

        [[nodiscard]] const Font& default_font() const;

        void add_font(std::unique_ptr<Font> font);

    private:
        FontManager();

        std::map<FontId, std::unique_ptr<Font>> fonts_;
        FontId default_font_id_;
    };
}
