//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iosfwd>
#include "SmoothMeshShader.hpp"
#include "VertexArrayDataBuilder.hpp"

namespace Tungsten
{
    using PositionNormal = SmoothMeshShader::VertexType;

    std::ostream& operator<<(std::ostream& os, const PositionNormal& p);

    std::ostream& operator<<(std::ostream& os, const VertexArrayData<PositionNormal>& buffer);

    void add_rect(VertexArrayDataBuilder<PositionNormal>& builder,
                  const Xyz::Rectangle3F& rect);

    void add_cube(VertexArrayDataBuilder<PositionNormal>& builder);
}
