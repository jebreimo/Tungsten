//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Matrix.hpp>

namespace Tungsten
{
    struct Camera
    {
        Xyz::Matrix4F view;
        Xyz::Matrix4F projection;
    };
}
