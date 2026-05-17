//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Command.hpp"

#include "Tungsten/Gl/GlTypes.hpp"

namespace Tungsten
{
    class DrawElementsCommand : public Command
    {
    public:
        DrawElementsCommand() = default;

        DrawElementsCommand(TopologyType topology, ElementIndexType type,
                            int32_t offset, int32_t index_count)
            : topology_{topology}, type_{type}, offset_{offset},
              index_count_{index_count}
        {
        }

        void run(const Camera& camera) override;

    private:
        TopologyType topology_;
        ElementIndexType type_;
        int32_t offset_;
        int32_t index_count_;
    };
} // Tungsten
