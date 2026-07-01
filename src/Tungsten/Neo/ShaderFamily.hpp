//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <vector>
#include "ShaderVariantKey.hpp"
#include "VertexLayout.hpp"

namespace Tungsten
{
    // A registered shader template from which concrete ShaderProgram variants are
    // compiled on demand (§14). A family is the GLSL source pair plus the ordered
    // list of feature flags it understands and the vertex format its attributes
    // expect; ResourceManager's ShaderLibrary owns the registered families and
    // turns a ShaderVariantKey into a compiled program.
    //
    // `features` is the single place that maps a ShaderVariantKey::defines bit to
    // its `#define` spelling: bit i set means `#define <features[i]>` is prepended
    // to the source before compiling. Keeping the mapping ordered here is what lets
    // the key stay a cheap-to-compare bitmask rather than a set of strings.
    //
    // `required_layout` is the vertex format every variant of this family expects;
    // it is validated against a mesh's VertexLayout when building the VAO and is
    // copied onto each compiled ShaderProgram.
    struct ShaderFamily
    {
        ShaderFamilyId id = 0;
        std::string vertex_source;
        std::string fragment_source;
        std::vector<std::string> features;
        VertexLayout required_layout;
    };
} // Tungsten