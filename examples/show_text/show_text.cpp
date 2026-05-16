//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <thread>
#include <Tungsten/TextRenderer2.hpp>
#include <Tungsten/Tungsten.hpp>

Xyz::Vector4F RED = {1.f, 0.f, 0.f, 1.f};
Xyz::Vector4F BLUE = {0.f, 0.f, 1.f, 1.f};
Xyz::Vector4F GREEN = {0.f, 0.f, 1.f, 1.f};
Xyz::Vector4F BLACK = {0.f, 0.f, 0.f, 1.f};
Xyz::Vector4F WHITE = {1.f, 1.f, 1.f, 1.f};

auto get_horizontal_alignment(float x)
    -> Tungsten::HorizontalAlignment
{
    if (x == 0)
        return Tungsten::HorizontalAlignment::LEFT;
    if (x == 1)
        return Tungsten::HorizontalAlignment::RIGHT;
    return Tungsten::HorizontalAlignment::CENTER;
}

auto get_horizontal_anchor(float x)
    -> Tungsten::HorizontalAnchor
{
    if (x == 0)
        return Tungsten::HorizontalAnchor::LEFT;
    if (x == 1)
        return Tungsten::HorizontalAnchor::RIGHT;
    return Tungsten::HorizontalAnchor::CENTER;
}

auto get_vertical_anchor(float y)
    -> Tungsten::VerticalAnchor
{
    if (y == 0)
        return Tungsten::VerticalAnchor::BOTTOM;
    if (y == 1)
        return Tungsten::VerticalAnchor::TOP;
    return Tungsten::VerticalAnchor::CENTER;
}

auto make_text_item(Tungsten::TextRenderer2& renderer,
                    std::shared_ptr<Tungsten::Font> font,
                    const Xyz::Vector4F& color,
                    const Xyz::Vector2F& position)
    -> std::shared_ptr<Tungsten::TextItem>
{
    auto item = std::make_shared<Tungsten::TextItem>("", std::move(font));
    item->set_color(color);
    item->set_horizontal_alignment(get_horizontal_alignment(position.x()));
    item->set_horizontal_anchor(get_horizontal_anchor(position.x()));
    item->set_vertical_anchor(get_vertical_anchor(position.y()));
    renderer.add_text(item);
    return item;
}

class ShowText : public Tungsten::EventLoop
{
public:
    explicit ShowText(Tungsten::SdlApplication& app)
        : EventLoop(app),
          text_manager_(std::make_shared<Tungsten::TextRenderer2>())
    {
        auto font = font_manager_.default_font();
        text_items_[0] = make_text_item(*text_manager_, font, RED, positions_[0]);
        text_items_[1] = make_text_item(*text_manager_, font, GREEN, positions_[1]);
        text_items_[2] = make_text_item(*text_manager_, font, BLUE, positions_[2]);
        text_items_[3] = make_text_item(*text_manager_, font, BLACK, positions_[3]);
        text_items_[4] = make_text_item(*text_manager_, font, WHITE, positions_[4]);
    }

    void on_update() override
    {
        const auto current_second = SDL_GetTicks() / 1000;
        if (current_second != second_)
        {
            second_ = current_second;
            const auto text = "Jan Erik Breimo\nNatasha Barrett\nTime: " + std::to_string(second_);
            for (const auto& item : text_items_)
            {
                item->set_text(text);
            }
            redraw();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    void on_draw() override
    {
        Tungsten::set_clear_color(0.4, 0.6, 0.8, 1);
        Tungsten::clear(Tungsten::ClearBits::COLOR_DEPTH);
        const auto viewport = application().viewport();
        Tungsten::set_viewport(viewport);
        for (size_t i = 0; i < 5; ++i)
        {
            text_items_[i]->set_position(viewport.normalized_to_pixel(positions_[i]));
        }

        const Tungsten::Camera camera(viewport, {}, {});
        text_manager_->prepare(camera);
        text_manager_->render(camera);

        Tungsten::set_ogl_tracing_enabled(false);
    }

private:
    Tungsten::FontManager font_manager_;
    std::shared_ptr<Tungsten::TextRenderer2> text_manager_;
    std::shared_ptr<Tungsten::TextItem> text_items_[5];
    std::array<Xyz::Vector2F, 5> positions_{
        Xyz::Vector2F(0.f, 0.f),
        Xyz::Vector2F(0.f, 1.f),
        Xyz::Vector2F(1.f, 0.f),
        Xyz::Vector2F(1.f, 1.f),
        Xyz::Vector2F(0.5f, 0.5f)
    };
    uint32_t second_ = UINT32_MAX;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("ShowText");
        app.parse_command_line_options(argc, argv);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<ShowText>();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
