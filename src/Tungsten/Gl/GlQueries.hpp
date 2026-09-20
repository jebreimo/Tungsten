//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <string>

namespace Tungsten
{
    /**
     * @file
     * The untyped OpenGL query and capability functions.
     *
     * These take a raw GL enum as an @c unsigned, so they cannot be called
     * without a GL_* constant and have no meaning for any other graphics API.
     * They are internal to the OpenGL backend for that reason: everything a
     * caller outside it needs is exposed as a typed function in
     * Gl/GlStateManagement.hpp or as a field of DeviceInfo.
     */

    [[nodiscard]] bool get_boolean_value(unsigned parameter_name);

    [[nodiscard]] float get_float_value(unsigned parameter_name);

    [[nodiscard]] int32_t get_int32_value(unsigned parameter_name);

    [[nodiscard]] int64_t get_int64_value(unsigned parameter_name);

    [[nodiscard]] std::string get_string_value(unsigned parameter_name);

    [[nodiscard]] bool is_enabled(unsigned capability);

    void set_enabled(unsigned capability, bool enabled);

    void enable(unsigned capability);

    void disable(unsigned capability);
}
