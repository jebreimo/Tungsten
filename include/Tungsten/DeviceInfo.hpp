//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <vector>

namespace Tungsten
{
    struct DeviceInfo
    {
        std::string vendor;
        std::string renderer;
        std::string version;
        std::string shader_language_version;
        std::vector<std::string> extensions;
    };

    DeviceInfo get_device_info();
}
