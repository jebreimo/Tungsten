//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL3/SDL_video.h>

/**
 * Owns the ImGui context and its SDL and OpenGL backends. Needs a current
 * GL context for as long as it lives.
 */
class ImGuiSession
{
public:
    ImGuiSession(SDL_Window* window, SDL_GLContext gl_context);

    ~ImGuiSession();

    ImGuiSession(const ImGuiSession&) = delete;
    ImGuiSession& operator=(const ImGuiSession&) = delete;

    /**
     * Starts a new ImGui frame. Call before building any controls.
     */
    void begin_frame();

    /**
     * Draws the controls built since begin_frame().
     */
    void end_frame();
};
