//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlParameters.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    GlVersion make_gl_version(GlVersionCode version_code)
    {
        switch (version_code)
        {
            #if defined(__EMSCRIPTEN__) || defined(__arm__)
        case GlVersionCode::ES_2:
        case GlVersionCode::CORE_3_1:
            return {SDL_GL_CONTEXT_PROFILE_ES, 2, 0};
            #else
        case GlVersionCode::ES_2:
        case GlVersionCode::CORE_3_1:
            return {SDL_GL_CONTEXT_PROFILE_CORE, 3, 1};
        case GlVersionCode::CORE_4_1:
            return {SDL_GL_CONTEXT_PROFILE_CORE, 4, 1};
            #endif
        default:
            TUNGSTEN_THROW("Unsupported OpenGL version.");
        }
    }

    GlVersion get_sdl_gl_version()
    {
        int p, ma, mi;
        if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &p) != 0
            || SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &ma) != 0
            || SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &mi) != 0)
        {
            THROW_SDL_ERROR();
        }

        return {SDL_GLprofile(p), ma, mi};
    }

    void set_sdl_gl_version(const GlVersion& version)
    {
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                                version.profile) != 0
            || SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,
                                   version.major_version) != 0
            || SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,
                                   version.minor_version) != 0)
        {
            THROW_SDL_ERROR();
        }
    }

    GlMultiSampling get_sdl_gl_multi_sampling()
    {
        int buffers, samples;
        if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers) != 0
            || SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples) != 0)
        {
            THROW_SDL_ERROR();
        }
        return {buffers, samples};
    }

    void set_sdl_gl_multi_sampling(const GlMultiSampling& multi_sampling)
    {
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,
                                multi_sampling.buffers) != 0
            || SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
                                   multi_sampling.samples) != 0)
        {
            THROW_SDL_ERROR();
        }
    }

    void set_sdl_gl_parameters(const GlParamaters& params)
    {
        if (params.version)
            set_sdl_gl_version(params.version);
        else
            set_sdl_gl_version(make_gl_version(GlVersionCode::ES_2));

        if (params.multi_sampling)
            set_sdl_gl_multi_sampling(params.multi_sampling);
    }
}
