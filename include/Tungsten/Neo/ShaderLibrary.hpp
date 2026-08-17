//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <functional>
#include <utility>
#include <vector>
#include "ResourceRefs.hpp"
#include "ShaderFamily.hpp"
#include "ShaderProgram.hpp"
#include "ShaderVariantKey.hpp"

namespace Tungsten
{
    /**
     * Compiles shader variants on demand and owns the family registry and variant
     * cache. Owned by ResourceManager, and the ONLY producer of
     * ShaderProgram values: it compiles a program, inserts it into the shader
     * pool through the injected inserter, and hands back a ShaderProgramRef, so
     * callers never build a ShaderProgram directly.
     *
     * Lazy compilation plus caching keeps the number of GL programs bounded even
     * though a family has many feature permutations (skinning, normal-mapping,
     * alpha-clip, ...): a Material selects a family plus a bitmask of features,
     * and identical selections resolve to one shared program.
     */
    class ShaderLibrary
    {
    public:
        /**
         * Moves a freshly compiled ShaderProgram into the shader pool and returns
         * its ref. Injected so the library need not know the pool's concrete type
         * (the same decoupling VaoCache uses for its arena/layout resolvers).
         */
        using ShaderInserter = std::function<ShaderProgramRef(ShaderProgram)>;

        explicit ShaderLibrary(ShaderInserter insert_shader);

        /**
         * Registers the family reached by `id`, replacing any family already at
         * that id. The id is the interned family name; it is stamped onto the
         * stored family.
         */
        void register_family(ShaderFamilyId id, ShaderFamily family);

        /**
         * Returns the program for `key`, compiling and caching it on first use.
         * Throws if the key names an unregistered family.
         */
        ShaderProgramRef register_variant(const ShaderVariantKey& key);

    private:
        [[nodiscard]] const ShaderFamily& get_family(ShaderFamilyId id) const;

        ShaderInserter insert_shader_;
        std::vector<ShaderFamily> families_;
        /**
         * The variant cache is a vector of (key, ref) pairs. It is searched
         * linearly on every variant request, so it is not suitable for large
         * numbers of variants. It is expected that the number of distinct
         * variants in a scene is small and bounded, so the linear search is
         * cheap and keeps the library simple.
         */
        std::vector<std::pair<ShaderVariantKey, ShaderProgramRef>> variant_cache_;
    };
} // Tungsten
