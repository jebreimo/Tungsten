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

    void on_startup(Tungsten::SdlApplication& app) override
    {
        text_renderer_.prepare_text("Jan Erik Breimo");
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto [w, h] = app.window_size();
        text_renderer_.draw_text({-1.0f, 0.0f}, {float(w), float(h)});
    }
private:
    Tungsten::TextRenderer text_renderer_;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("Circles", std::make_unique<ShowText>());
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
