//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlSampler.hpp"

#include <vector>

#include "Tungsten/ExceptionHelpers.hpp"
#include "Tungsten/Gl/IOglWrapper.hpp"
#include "GlTypeConversion.hpp"

namespace Tungsten
{
    void SamplerDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().delete_samplers(1, &id);
        THROW_IF_GL_ERROR();
    }

    SamplerHandle generate_sampler()
    {
        uint32_t id;
        get_ogl_wrapper().gen_samplers(1, &id);
        THROW_IF_GL_ERROR();
        return SamplerHandle(id);
    }

    void generate_samplers(std::span<SamplerHandle> samplers)
    {
        std::vector<uint32_t> ids(samplers.size());
        get_ogl_wrapper().gen_samplers(static_cast<GLsizei>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < samplers.size(); ++i)
            samplers[i] = SamplerHandle(ids[i]);
    }

    void bind_sampler(uint32_t unit, uint32_t sampler)
    {
        get_ogl_wrapper().bind_sampler(unit, sampler);
        THROW_IF_GL_ERROR();
    }

    uint32_t bound_sampler(uint32_t unit)
    {
        GLint sampler;
        get_ogl_wrapper().get_integer_i(GL_SAMPLER_BINDING, unit, &sampler);
        THROW_IF_GL_ERROR();
        return static_cast<uint32_t>(sampler);
    }

    void set_sampler_parameters(uint32_t sampler, const SamplerDescriptor& descriptor)
    {
        auto& gl = get_ogl_wrapper();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_MIN_FILTER, to_ogl_texture_min_filter(descriptor.min_filter, descriptor.mip_filter));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_MAG_FILTER, to_ogl_texture_mag_filter(descriptor.mag_filter));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_WRAP_S, to_ogl_texture_wrap_mode(descriptor.address_mode_u));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_WRAP_T, to_ogl_texture_wrap_mode(descriptor.address_mode_v));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_WRAP_R, to_ogl_texture_wrap_mode(descriptor.address_mode_w));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_COMPARE_FUNC, to_ogl_texture_compare_func(descriptor.compare_function));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_i(sampler, GL_TEXTURE_COMPARE_MODE, to_ogl_texture_compare_mode(descriptor.compare_function));
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_f(sampler, GL_TEXTURE_MIN_LOD, descriptor.min_lod);
        THROW_IF_GL_ERROR();
        gl.sampler_parameter_f(sampler, GL_TEXTURE_MAX_LOD, descriptor.max_lod);
        THROW_IF_GL_ERROR();
    }
}
