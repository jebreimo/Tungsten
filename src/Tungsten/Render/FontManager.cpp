//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/FontManager.hpp"
#include "Tungsten/Render/TextRenderer/Monaco32.hpp"

namespace Tungsten
{
    FontManager::FontManager()
    {
        auto default_font = get_monaco_32();
        default_font_id_ = default_font->identifier;
        add_font(std::move(default_font));
    }

    void FontManager::add_font(std::shared_ptr<Font> font)
    {
        fonts_.insert({font->identifier, std::move(font)});
    }

    const std::shared_ptr<Font>& FontManager::default_font() const
    {
        return fonts_.find(default_font_id_)->second;
    }
}
