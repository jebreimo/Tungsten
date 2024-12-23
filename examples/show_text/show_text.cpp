//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <thread>
#include <Tungsten/Tungsten.hpp>
#include <Yconvert/Convert.hpp>


class ShowText : public Tungsten::EventLoop
{
public:
    explicit ShowText(Tungsten::SdlApplication& app)
        : EventLoop(app),
          text_renderer_(Tungsten::FontManager::instance().default_font())
    {}

    void on_update() override
    {
        auto current_second = SDL_GetTicks() / 1000;
        if (current_second != second_)
        {
            second_ = current_second;
            redraw();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    void on_draw() override
    {
        glClearColor(0.4, 0.6, 0.8, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto [w, h] = application().window_size();
        auto screen_size = Xyz::Vector2F(float(w), float(h));
        auto u8text = "Jan Erik Breimo\nNatasha Barrett\nTime: " + std::to_string(second_);

        auto text = Yconvert::convert_to<std::u32string>(
            u8text,
            Yconvert::Encoding::UTF_8,
            Yconvert::Encoding::UTF_32_NATIVE,
            Yconvert::ErrorPolicy::REPLACE
        );

        auto size = Tungsten::get_size(text, text_renderer_.font()) * 2.f / screen_size;
        text_renderer_.draw(text, {-size[0] / 2, -size[1] / 2}, screen_size,
                            {.color = {0xFF, 0, 0, 0xFF}});
        text_renderer_.draw(text, {-1.f, 1.f - size[1]}, screen_size,
                            {.color = {0, 0xFF, 0, 0xFF}});
        text_renderer_.draw(text, {-1.f, -1.f}, screen_size,
                            {.color = {0, 0, 0xFF, 0xFF}});
        text_renderer_.draw(text, {1.f - size[0], -1.f}, screen_size,
                            {.color = {0xFF, 0, 0xFF, 0xFF}});
        text_renderer_.draw(text, {1.f - size[0], 1.f - size[1]},
                            screen_size, {.color = {0xFF, 0xFF, 0xFF, 0xFF}});
    }
private:
    Tungsten::TextRenderer text_renderer_;
    uint32_t second_ = UINT32_MAX;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("ShowText");
        app.parse_command_line_options(argc, argv);
        app.run<ShowText>();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
