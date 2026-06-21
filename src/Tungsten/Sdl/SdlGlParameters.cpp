//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Sdl/SdlGlParameters.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    bool SdlGlVersion::is_es() const
    {
        return profile == SDL_GL_CONTEXT_PROFILE_ES;
    }

    bool SdlGlVersion::is_core() const
    {
        return profile == SDL_GL_CONTEXT_PROFILE_CORE;
    }

    SdlGlVersionCode get_sdl_gl_version_code()
    {
        auto [profile, maj, min] = get_sdl_gl_version();
        if (profile == SDL_GL_CONTEXT_PROFILE_ES)
        {
            if (maj == 2)
                return SdlGlVersionCode::ES_2;
            if (maj == 3)
                return SdlGlVersionCode::ES_3;
        }
        else if (profile == SDL_GL_CONTEXT_PROFILE_CORE)
        {
            if (maj == 3 && min >= 1)
                return SdlGlVersionCode::CORE_3_1;
            if (maj == 4 && min >= 1)
                return SdlGlVersionCode::CORE_4_1;
        }
        TUNGSTEN_THROW("Unsupported OpenGL version.");
    }

    SdlGlVersion make_gl_version(SdlGlVersionCode version_code)
    {
        switch (version_code)
        {
#if defined(__EMSCRIPTEN__) || defined(__arm__)
        case SdlGlVersionCode::ES_2:
        case SdlGlVersionCode::CORE_3_1:
            return {SDL_GL_CONTEXT_PROFILE_ES, 2, 0};
        case SdlGlVersionCode::ES_3:
            return {SDL_GL_CONTEXT_PROFILE_ES, 3, 0};
#else
        case SdlGlVersionCode::ES_2:
        case SdlGlVersionCode::ES_3:
        case SdlGlVersionCode::CORE_3_1:
            return {SDL_GL_CONTEXT_PROFILE_CORE, 3, 3};
        case SdlGlVersionCode::CORE_4_1:
            return {SDL_GL_CONTEXT_PROFILE_CORE, 4, 1};
#endif
        default:
            TUNGSTEN_THROW("Unsupported OpenGL version.");
        }
    }

    SdlGlVersion get_sdl_gl_version()
    {
        int p, ma, mi;
        if (!SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &p)
            || !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &ma)
            || !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &mi))
        {
            THROW_SDL_ERROR();
        }

        return {SDL_GLProfile(p), ma, mi};
    }

    void set_sdl_gl_version(const SdlGlVersion& version)
    {
        if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                                 int(version.profile))
            || !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,
                                    version.major_version)
            || !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,
                                    version.minor_version))
        {
            THROW_SDL_ERROR();
        }
    }

    SdlGlMultiSampling get_sdl_gl_multi_sampling()
    {
        int buffers, samples;
        if (!SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers)
            || !SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples))
        {
            THROW_SDL_ERROR();
        }
        return {buffers, samples};
    }

    void set_sdl_gl_multi_sampling(const SdlGlMultiSampling& multi_sampling)
    {
        if (!SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,
                                multi_sampling.buffers)
            || !SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
                                   multi_sampling.samples))
        {
            THROW_SDL_ERROR();
        }
    }

    void set_sdl_gl_parameters(const GlParameters& params)
    {
        if (params.version)
            set_sdl_gl_version(params.version);
        else
            set_sdl_gl_version(make_gl_version(SdlGlVersionCode::ES_3));

        if (params.multi_sampling)
            set_sdl_gl_multi_sampling(params.multi_sampling);
    }
}
