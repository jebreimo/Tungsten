//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-09.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include "Shape2D.hpp"
#include "SceneFader.hpp"

class fading_blob : public Tungsten::EventLoop
{
public:
    explicit fading_blob(Tungsten::SdlApplication& app)
        : EventLoop(app),
          fader_(app.window_size())
    {
        Shape2DRenderer::Buffer buffer;
        Tungsten::ArrayBufferBuilder builder(buffer);
        builder.add_vertex({-1, -1})
            .add_vertex({1, -1})
            .add_vertex({1, 1})
            .add_vertex({-1, 1});
        builder.add_indexes(0, 1, 2)
            .add_indexes(0, 2, 3);
        rectangle_ = renderer_.create_shape(buffer);
        set_swap_interval(app, Tungsten::SwapInterval::VSYNC);
    }

    bool on_event(const SDL_Event& event) override
    {
        if (event.type == SDL_WINDOWEVENT
            && event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            glFinish();
            glViewport(0, 0, event.window.data1, event.window.data2);
            fader_.set_window_size({event.window.data1, event.window.data2});
            return true;
        }
        return false;
    }

    void on_draw() override
    {
        fader_.draw_previous_scene(0.97);

        float angle = std::fmod(static_cast<float>(SDL_GetTicks()) * 0.001f, 2 * Xyz::Constants<float>::PI);
        renderer_.set_model_view_matrix(
            Xyz::affine::rotate2(angle)
            * Xyz::affine::translate2(0.5f, 0.f)
            * Xyz::affine::scale2(0.2f, 0.2f)
            );
        renderer_.draw(rectangle_);

        fader_.render_scene();

        redraw();
    }

private:
    Shape2DRenderer renderer_;
    SceneFader fader_;
    Shape2D rectangle_;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("Something");
        app.parse_command_line_options(argc, argv);
        app.run<fading_blob>();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
