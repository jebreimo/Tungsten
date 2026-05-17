//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "DrawElementsCommand.hpp"

#include "Tungsten/Gl/GlRendering.hpp"

namespace Tungsten
{
    void DrawElementsCommand::run(const Camera& /*camera*/)
    {
        draw_elements(topology_, type_, offset_, index_count_);
    }
}
