//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/FontManager.hpp"
#include "Monaco32.hpp"

#include <Yimage/WritePng.hpp>

namespace Tungsten
{
    FontManager& FontManager::instance()
    {
        static FontManager manager;
        return manager;
    }

    FontManager::FontManager()
    {
        auto default_font = get_monaco_32();
        default_font_id_ = default_font->identifier;
        Yimage::write_png("image.png", default_font->image);
        add_font(std::move(default_font));
    }

    void FontManager::add_font(std::unique_ptr<Font> font)
    {
        fonts_.insert({font->identifier, std::move(font)});
    }

    const Font& FontManager::default_font() const
    {
        return *fonts_.find(default_font_id_)->second;
    }
}
