//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Tungsten/Tungsten.hpp>

class ShowText : public Tungsten::EventLoop
{
public:
    explicit ShowText()
        : text_renderer_(Tungsten::FontManager::instance().default_font())
    {}

    void on_update(Tungsten::SdlApplication& app) override
    {
        auto current_second = SDL_GetTicks() / 1000;
        if (current_second != second_)
        {
            second_ = current_second;
            text_ = "Jan Erik Breimo\nNatasha Barrett\nTime: " + std::to_string(second_);
            text_renderer_.prepare_text(text_);
            redraw();
        }
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto [w, h] = app.window_size();
        auto screen_size = Xyz::Vector2F(float(w), float(h));
        auto size = text_renderer_.get_size() * 2.f / screen_size;
        text_renderer_.set_color({0xFF, 0, 0, 0xFF});
        text_renderer_.draw_text({-size[0] / 2, -size[1] / 2}, screen_size);
        text_renderer_.set_color({0, 0xFF, 0, 0xFF});
        text_renderer_.draw_text({-1.f, 1.f - size[1]}, screen_size);
        text_renderer_.set_color({0, 0, 0xFF, 0xFF});
        text_renderer_.draw_text({-1.f, -1.f}, screen_size);
        text_renderer_.set_color({0xFF, 0, 0xFF, 0xFF});
        text_renderer_.draw_text({1.f - size[0], -1.f}, screen_size);
        text_renderer_.set_color({0xFF, 0xFF, 0xFF, 0xFF});
        text_renderer_.draw_text({1.f - size[0], 1.f - size[1]}, screen_size);
    }
private:
    Tungsten::TextRenderer text_renderer_;
    uint32_t second_ = UINT32_MAX;
    std::string text_;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("ShowText", std::make_unique<ShowText>());
        app.parse_command_line_options(argc, argv);
        app.run();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
