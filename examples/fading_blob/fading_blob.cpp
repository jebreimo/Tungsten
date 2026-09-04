//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-09.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <cmath>
#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include "Shape2D.hpp"
#include "SceneFader.hpp"

class fading_blob : public Tungsten::EventLoop
{
public:
    explicit fading_blob(Tungsten::SdlApplication& app)
        : EventLoop(app),
          fader_(app.window_size()),
          window_size_(app.window_size())
    {
        const std::vector<Xyz::Vector2F> vertexes = {
            {-1, -1},
            {1, -1},
            {1, 1},
            {-1, 1}
        };
        const std::vector<uint16_t> indexes = {
            0, 1, 2,
            0, 2, 3
        };
        rectangle_ = renderer_.create_shape(vertexes, indexes);
        // set_swap_interval(app, Tungsten::SwapInterval::VSYNC);
    }

    void on_draw() override
    {
        const auto viewport = application().viewport();
        Tungsten::set_viewport(viewport);
        const Tungsten::Size2I size = {int(viewport.size[0]), int(viewport.size[1])};
        if (size != window_size_)
        {
            window_size_ = size;
            fader_.set_window_size(size);
        }

        fader_.draw_previous_scene(1.f / 256.f);

        float angle = std::fmod(static_cast<float>(SDL_GetTicks()) * 0.001f, 2 * Xyz::Constants<float>::PI);
        renderer_.set_model_view_matrix(
            Xyz::affine::rotate2(angle)
            * Xyz::affine::translate2(0.5f, 0.f)
            * Xyz::affine::scale2(0.2f, 0.2f)
            );
        renderer_.draw(rectangle_);

        fader_.render_scene();

        Tungsten::set_ogl_tracing_enabled(false);

        redraw();
    }

private:
    Shape2DRenderer renderer_;
    SceneFader fader_;
    Shape2D rectangle_;
    Tungsten::Size2I window_size_;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("Fading blob");
        app.parse_command_line_options(argc, argv);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<fading_blob>();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
