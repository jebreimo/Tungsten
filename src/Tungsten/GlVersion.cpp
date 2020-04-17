//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVersion.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    GlVersion getDefaultGlVersion(GlVersionCode versionCode)
    {
        switch (versionCode)
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

    GlVersion getSdlGlVersion()
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

    void setSdlGlVersion(GlVersion version)
    {
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                                version.profile) != 0
            || SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,
                                   version.majorVersion) != 0
            || SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,
                                   version.minorVersion) != 0)
        {
            THROW_SDL_ERROR();
        }
    }
}
