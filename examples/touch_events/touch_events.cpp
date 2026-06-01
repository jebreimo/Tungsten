//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-12-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <ranges>
#include <thread>
#include <Tungsten/Tungsten.hpp>
#include <Yconvert/Convert.hpp>
#include "RingBuffer.hpp"
#include "../show_text/Debug.hpp"

class TextScroller : public Tungsten::Renderable
{
public:
    explicit TextScroller(std::shared_ptr<Tungsten::Font> font)
        : font_(std::move(font)),
          text_renderer_(std::make_shared<Tungsten::TextRenderer>())
    {
    }

    void add_text(const std::string& text)
    {
        texts_.push_back(text);
        auto item = std::make_unique<Tungsten::TextItem>(text, font_);
        item->set_color({1.f, 1.f, 1.f, 1.f});
        item->set_horizontal_alignment(Tungsten::HorizontalAlignment::LEFT);
        item->set_horizontal_anchor(Tungsten::HorizontalAnchor::LEFT);
        item->set_vertical_anchor(Tungsten::VerticalAnchor::BOTTOM);
        if (text_item_ids_.full())
            text_renderer_->remove_text_item(text_item_ids_.front());
        text_item_ids_.push_back(text_renderer_->add_text_item(std::move(item)));
        dirty_ = true;
    }

    void prepare(const Tungsten::Camera& camera) override
    {
        if (!dirty_)
            return;

        const auto& viewport = camera.viewport();
        // Calculate number of visible items
        size_t visible_count = 0;
        float height = 0;
        for (const auto id : std::ranges::reverse_view(text_item_ids_))
        {
            const auto* item = text_renderer_->get_text_item(id);
            const auto text_size = item->calc_size();
            visible_count++;
            height += text_size.y() + (visible_count ? line_gap_ : 0);
            if (height >= viewport.size.y())
            {
                height = viewport.size.y();
                break;
            }
        }

        float y = 0;
        for (size_t i = 0; i < visible_count; ++i)
        {
            auto* item = text_renderer_->get_text_item(text_item_ids_[text_item_ids_.size() - i - 1]);
            const auto text_size = item->calc_size();
            item->set_position({0, y});
            y += text_size.y() + line_gap_;
        }
        for (size_t i = visible_count; i < text_item_ids_.size(); ++i)
        {
            auto* item = text_renderer_->get_text_item(text_item_ids_[text_item_ids_.size() - i - 1]);
            item->set_visible(false);
        }
        text_renderer_->prepare(camera);
    }

    void render(const Tungsten::Camera& camera) const override
    {
        text_renderer_->render(camera);
    }

private:
    std::shared_ptr<Tungsten::Font> font_;
    std::shared_ptr<Tungsten::TextRenderer> text_renderer_;
    std::vector<std::string> texts_;
    Chorasmia::RingBuffer<size_t, 100> text_item_ids_;
    float line_gap_ = 2.f;
    bool dirty_ = false;
};

class EventLoop : public Tungsten::EventLoop
{
public:
    explicit EventLoop(Tungsten::SdlApplication& app)
        : Tungsten::EventLoop(app),
          text_scroller_(font_manager_.default_font())
    {
        // set_swap_interval(application(), Tungsten::SwapInterval::ADAPTIVE_VSYNC_OR_VSYNC);
    }

    void on_finger_event(const SDL_TouchFingerEvent& event)
    {
        JEB_CHECKPOINT();
        std::ostringstream ss;
        switch (event.type)
        {
        case SDL_EVENT_FINGER_DOWN: ss << "finger down: ";
            break;
        case SDL_EVENT_FINGER_UP: ss << "finger up: ";
            break;
        case SDL_EVENT_FINGER_MOTION: ss << "finger motion: ";
            break;
        case SDL_EVENT_FINGER_CANCELED: ss << "finger canceled: ";
            break;
        default: ss << "unknown: ";
            break;
        }
        ss << "id " << event.fingerID << " " << event.x << " " << event.y
            << " " << event.dx << " " << event.dy
            << " " << event.pressure;
        text_scroller_.add_text(ss.str());
        redraw();
    }

    void on_mouse_wheel(const SDL_MouseWheelEvent& event)
    {
        std::ostringstream ss;
        ss << "wheel: " << event.x << " " << event.y;
#if SDL_VERSION_ATLEAST(3, 2, 12)
        ss << " " << event.integer_x << " " << event.integer_y;
#endif
        text_scroller_.add_text(ss.str());
        redraw();
    }

    bool on_event(const SDL_Event& event) override
    {
        std::string msg;
        switch (event.type)
        {
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
        case SDL_EVENT_FINGER_CANCELED:
            on_finger_event(event.tfinger);
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            on_mouse_wheel(event.wheel);
            break;
        default:
            return false;
        }
        return true;
    }

    void on_draw() override
    {
        const auto viewport = application().viewport();
        Tungsten::set_viewport(viewport);
        const Tungsten::Camera camera(viewport, {}, {});
        text_scroller_.prepare(camera);

        Tungsten::set_clear_color(0.4, 0.6, 0.8, 1);
        Tungsten::clear(Tungsten::ClearBits::COLOR_DEPTH);

        text_scroller_.render(camera);
    }

private:
    Tungsten::FontManager font_manager_;
    TextScroller text_scroller_;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("touch_events");
        app.parse_command_line_options(argc, argv);
        app.set_event_loop_mode(Tungsten::EventLoopMode::WAIT_FOR_EVENTS);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<EventLoop>();
    }
    catch (std::exception& ex)
    {
        Tungsten::print_exception(ex);
        return 1;
    }
    return 0;
}
