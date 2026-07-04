//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/ShaderLibrary.hpp"
#include <string>
#include "Tungsten/Gl/GlProgram.hpp"
#include "Tungsten/Gl/GlTypes.hpp"
#include "Tungsten/Render/ShaderProgramBuilder.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Neo/UboBindings.hpp"

namespace Tungsten
{
    namespace
    {
        // Turns a defines bitmask into a block of #define lines: bit i selects
        // features[i]. Bits without a matching feature (and beyond bit 31) are
        // ignored. The family's ordered feature list is the single place a bit
        // maps to its #define spelling (§14).
        std::string build_define_block(const std::vector<std::string>& features,
                                       uint32_t defines)
        {
            std::string block;
            for (size_t i = 0; i < features.size() && i < 32; ++i)
            {
                if (defines & (uint32_t{1} << i))
                {
                    block += "#define ";
                    block += features[i];
                    block += '\n';
                }
            }
            return block;
        }

        // Injects the #define block into a shader source. GLSL requires #version
        // to stay the first line, so the block goes on the line *after* it rather
        // than being literally prepended (which would push #version off the first
        // line and fail to compile); sources without a #version get a plain
        // prepend.
        std::string with_defines(const std::string& source,
                                 const std::string& defines)
        {
            if (defines.empty())
                return source;

            if (source.starts_with("#version"))
            {
                const auto eol = source.find('\n');
                if (eol != std::string::npos)
                {
                    return source.substr(0, eol + 1) + defines
                           + source.substr(eol + 1);
                }
            }
            return defines + source;
        }

        // Wires a freshly linked program's uniform blocks to the fixed
        // binding points (§4). GLSL ES 3.00 cannot declare bindings in the
        // source, so the convention is applied here, once per compiled
        // variant. A family that lacks one of the blocks (e.g. an unlit
        // shader without MaterialBlock) is simply skipped for that block.
        void apply_ubo_bindings(uint32_t program_id)
        {
            static constexpr std::pair<const char*, uint32_t> bindings[] = {
                {"PerFrame", PER_FRAME_UBO_BINDING},
                {"MaterialBlock", PER_MATERIAL_UBO_BINDING},
                {"PerDraw", PER_DRAW_UBO_BINDING},
            };
            for (const auto& [name, binding] : bindings)
            {
                const auto index = get_uniform_block_index(program_id, name);
                if (index != INVALID_UNIFORM_BLOCK_INDEX)
                    set_uniform_block_binding(program_id, index, binding);
            }
        }
    }

    ShaderLibrary::ShaderLibrary(ShaderInserter insert_shader)
        : insert_shader_(std::move(insert_shader))
    {}

    void ShaderLibrary::register_family(ShaderFamilyId id, ShaderFamily family)
    {
        family.id = id;
        for (auto& existing : families_)
        {
            if (existing.id == id)
            {
                existing = std::move(family);
                return;
            }
        }
        families_.push_back(std::move(family));
    }

    ShaderProgramRef ShaderLibrary::register_variant(const ShaderVariantKey& key)
    {
        for (const auto& [cached_key, ref] : variant_cache_)
        {
            if (cached_key == key)
                return ref;
        }

        const ShaderFamily& family = get_family(key.family);
        const std::string defines = build_define_block(family.features,
                                                       key.defines);

        ShaderProgramBuilder builder;
        builder.add_shader(ShaderType::VERTEX,
                           with_defines(family.vertex_source, defines));
        builder.add_shader(ShaderType::FRAGMENT,
                           with_defines(family.fragment_source, defines));

        ShaderProgram program;
        program.gl_handle = builder.build();
        apply_ubo_bindings(program.gl_handle.id());
        program.variant_key = key;
        program.required_layout = family.required_layout;

        ShaderProgramRef ref = insert_shader_(std::move(program));
        variant_cache_.emplace_back(key, ref);
        return ref;
    }

    const ShaderFamily& ShaderLibrary::get_family(ShaderFamilyId id) const
    {
        for (const auto& family : families_)
        {
            if (family.id == id)
                return family;
        }
        TUNGSTEN_THROW("ShaderLibrary: no shader family registered for id.");
    }
} // Tungsten