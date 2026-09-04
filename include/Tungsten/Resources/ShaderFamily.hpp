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
#include "ResourceRefs.hpp"
#include "VertexAttribute.hpp"
#include "ShaderVariantKey.hpp"

namespace Tungsten
{
    /**
     * A registered shader template from which concrete ShaderProgram variants are
     * compiled on demand. A family is the GLSL source pair plus the ordered
     * list of feature flags it understands and the vertex format its attributes
     * expect; ResourceManager's ShaderLibrary owns the registered families and
     * turns a ShaderVariantKey into a compiled program.
     */
    struct ShaderFamily
    {
        ShaderFamilyId id = 0;
        std::string vertex_source;
        std::string fragment_source;
        /**
         * A list of flags that enable optional features supported
         * by the shader. Each flag corresponds to a boolean *define* in the
         * shader's code. The @a defines member of ShaderVariantKeys is where
         * these flags are controlled by the program – the first bit enables
         * the first flag, the second enables the second and so on.
         */
        std::vector<std::string> features;
        /**
         * A list of the names of the sampler uniforms in the shader, in
         * texture-unit order: sampler i samples unit i, which is the unit
         * the renderer binds a material's texture i to.
         */
        std::vector<std::string> samplers;
        /**
         * The semantics every variant of this family reads.
         */
        AttributeSemanticMask required_attributes = 0;
    };
} // Tungsten
