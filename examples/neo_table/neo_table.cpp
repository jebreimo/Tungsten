//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// Neo's text path: a table of labels that never change beside values that
// change every frame, drawn through the ordinary scene-graph pipeline.
//
//     TextSystem::update(scene)        <- compiles text into meshes/materials
//     scene.resolve_transforms()
//     SnapshotBuilder::build(...)      <- knows nothing about text
//     Renderer::render(...)            <- knows nothing about text
//
// The three properties the design is built around, all visible here:
//
//  * The example assigns *every* value cell on *every* frame. TextComponent
//    carries no dirty flags — TextSystem diffs each component against what it
//    last compiled — so a cell whose text happens to be unchanged (FPS, or
//    STATUS, which only move about once a second) costs nothing at all. Only
//    the cells whose digits actually differ are re-tessellated.
//
//  * The whole table slides and breathes, driven by one transform on its root
//    node. Moving or scaling text is free: position lives in the node and
//    reaches the shader through the per-draw UBO, so not one vertex is
//    rewritten.
//
//  * SINE's cell turns red when it goes negative. Colour lives in the
//    material, not in the vertex data, so a colour change re-resolves a
//    MaterialRef and touches no geometry either. Every black cell in the
//    table shares one material and is bound once for all of them.

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>

namespace
{
    using namespace Tungsten;

    // The table is laid out in the font's own units, which for the builtin
    // Monaco 32 are pixels at the size it was rasterized for. The camera below
    // makes one world unit one pixel, and the root node's scale then sizes the
    // whole table at once.
    constexpr float ROW_HEIGHT = 44.0f;
    constexpr float VALUE_COLUMN = 340.0f;
    constexpr float TABLE_MARGIN = 48.0f;

    const Xyz::Vector4F INK = {0.10f, 0.11f, 0.14f, 1.0f};
    const Xyz::Vector4F MUTED = {0.42f, 0.45f, 0.52f, 1.0f};
    const Xyz::Vector4F ACCENT = {0.05f, 0.32f, 0.52f, 1.0f};
    const Xyz::Vector4F ALERT = {0.74f, 0.14f, 0.14f, 1.0f};

    template <typename T>
    std::string format(const char* pattern, T value)
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), pattern, value);
        return buffer;
    }

    Transform at(float x, float y)
    {
        Transform transform;
        transform.translation = {x, y, 0};
        return transform;
    }

    class NeoTableLoop : public EventLoop
    {
    public:
        explicit NeoTableLoop(SdlApplication& app)
            : EventLoop(app),
              text_system_(resources_),
              renderer_(resources_)
        {
            const std::shared_ptr<const Font> font =
                font_manager_.default_font();

            // Four styles, each shared by every cell that uses it. A style is
            // immutable, so cells using one cannot drift apart, and TextSystem
            // can tell whether a cell's style changed by comparing pointers.
            title_style_ = make_text_style({
                .font = font,
                .color = ACCENT,
                .horizontal_anchor = HorizontalAnchor::LEFT
            });
            label_style_ = make_text_style({
                .font = font,
                .color = MUTED,
                .horizontal_anchor = HorizontalAnchor::LEFT
            });
            // Values are right-anchored so their digits line up in a column
            // however wide they get. The anchor is baked into the glyph
            // positions when the text is tessellated, so it costs nothing per
            // frame and needs no uniform.
            value_style_ = make_text_style({
                .font = font,
                .color = INK,
                .horizontal_alignment = HorizontalAlignment::RIGHT,
                .horizontal_anchor = HorizontalAnchor::RIGHT
            });
            footer_style_ = make_text_style({
                .font = font,
                .color = MUTED,
                .line_gap = 0.3f,
                .horizontal_anchor = HorizontalAnchor::LEFT
            });

            table_ = scene_.add_node();

            auto title = table_.add_child();
            title.set_local_transform(at(0, 1.4f * ROW_HEIGHT));
            title.add(TextComponent{.text = "Neo text - live table",
                                    .style = title_style_});

            // Each row is a child of the table root, and its label and value
            // are children of the row. Nothing but the root's transform is
            // touched afterwards; the resolve pass composes the rest (§2).
            for (const char* label : {"FRAME", "ELAPSED", "FPS", "SINE",
                                      "STATUS"})
            {
                rows_.push_back(add_row(int(rows_.size()), label));
            }

            auto footer = table_.add_child();
            footer.set_local_transform(at(0, -float(rows_.size()) * ROW_HEIGHT
                                             - 0.6f * ROW_HEIGHT));
            // One component is one text run however many lines it has, so this
            // is a single mesh and a single draw.
            footer.add(TextComponent{
                .text = "every value is assigned every frame;\n"
                        "only changed digits are re-tessellated",
                .style = footer_style_});

            // A 2D scene is nodes at z = 0 seen by an orthographic camera
            // (§8). The camera sits in front of that plane looking down its
            // node's -z axis, which is where everything directional points.
            camera_ = scene_.add_node();
            camera_.set_local_transform(at2(0, 0, 100));
            camera_.add(CameraComponent{
                .mode = ProjectionMode::ORTHOGRAPHIC,
                .near_plane = 1.0f,
                .far_plane = 200.0f,
                .aspect = app.viewport().aspect_ratio()
            });

            std::cout << get_device_info() << '\n';
            set_swap_interval(app, SwapInterval::VSYNC);
        }

        void on_update() override
        {
            const auto ticks = SDL_GetTicks();
            const auto elapsed = float(ticks - start_ticks_) / 1000.0f;

            // Smoothed so the reading is legible rather than a blur; it then
            // only changes every few frames, which is the point — the cell
            // below is assigned regardless and TextSystem does nothing when
            // the digits come out the same.
            if (ticks != last_ticks_)
            {
                const auto dt = float(ticks - last_ticks_) / 1000.0f;
                fps_ = fps_ == 0.0f ? 1.0f / dt : 0.92f * fps_ + 0.08f / dt;
                last_ticks_ = ticks;
            }

            const float sine = std::sin(1.7f * elapsed);

            set_value(0, format("%llu", static_cast<unsigned long long>(frame_)));
            set_value(1, format("%.1f s", elapsed));
            set_value(2, format("%.0f", fps_));
            set_value(3, format("%+.3f", sine));
            set_value(4, int(elapsed) % 2 ? "WAIT" : "OK");

            // The one cell that changes colour. Assigning the same override
            // again is a no-op — TextSystem compares it against what it built
            // — so this runs every frame and only acts at the zero crossings.
            auto& sine_cell = rows_[3].value.get<TextComponent>();
            if (sine < 0.0f)
                sine_cell.color_override = ALERT;
            else
                sine_cell.color_override.reset();

            // Blink one row, to show that visibility is not a rebuild either.
            rows_[4].value.get<TextComponent>().visible =
                int(elapsed * 2.0f) % 2 == 0;

            // The whole table on one transform: a slow drift and a breath of
            // scale. Every glyph in the table moves, and none is re-uploaded.
            Transform table;
            table.translation = {origin_.x() + 12.0f * std::sin(0.6f * elapsed),
                                 origin_.y(), 0.0f};
            const float scale = 0.95f + 0.05f * std::sin(0.9f * elapsed);
            table.scale = {scale, scale, 1.0f};
            table_.set_local_transform(table);
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            set_viewport(viewport);
            set_clear_color({0.97f, 0.96f, 0.94f, 1.0f});
            clear(ClearBits::COLOR_DEPTH);

            // One world unit per pixel: the orthographic half-height is half
            // the viewport's, so the visible volume is exactly the window in
            // pixels, with the origin at its centre.
            auto& camera = camera_.get<CameraComponent>();
            camera.ortho_size = viewport.size[1] / 2.0f;
            camera.aspect = viewport.aspect_ratio();
            // Anchor the table to the window's top-left corner rather than to
            // the centre the camera puts the origin at.
            origin_ = {-viewport.size[0] / 2.0f + TABLE_MARGIN,
                       viewport.size[1] / 2.0f - 2.0f * TABLE_MARGIN};

            resources_.begin_frame(frame_);

            // Before the resolve, so the meshes and bounds it publishes are in
            // place for the extraction pass that follows.
            text_system_.update(scene_);
            scene_.resolve_transforms();

            auto& snapshots = scene_.snapshots();
            builder_.build(scene_, camera_.id(), snapshots.back());
            snapshots.swap();
            renderer_.render(snapshots.front());

            // Single-threaded: the frame just drawn is complete (§11).
            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        struct Row
        {
            NodeHandle label;
            NodeHandle value;
        };

        static Transform at2(float x, float y, float z)
        {
            Transform transform;
            transform.translation = {x, y, z};
            return transform;
        }

        Row add_row(int index, const std::string& label_text)
        {
            auto row = table_.add_child();
            row.set_local_transform(at(0, -float(index) * ROW_HEIGHT));

            auto label = row.add_child();
            label.add(TextComponent{.text = label_text,
                                    .style = label_style_});

            auto value = row.add_child();
            value.set_local_transform(at(VALUE_COLUMN, 0));
            value.add(TextComponent{.style = value_style_});

            return {label, value};
        }

        // Assigned unconditionally: the component is the source of truth and
        // TextSystem decides whether anything has to happen. Reached through
        // the handle rather than a cached reference, because the component
        // arrays move their contents as they grow.
        void set_value(size_t row, std::string text)
        {
            rows_[row].value.get<TextComponent>().text = std::move(text);
        }

        ResourceManager resources_;
        FontManager font_manager_;
        Scene scene_;
        TextSystem text_system_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;

        std::shared_ptr<const TextStyle> title_style_;
        std::shared_ptr<const TextStyle> label_style_;
        std::shared_ptr<const TextStyle> value_style_;
        std::shared_ptr<const TextStyle> footer_style_;

        NodeHandle table_;
        NodeHandle camera_;
        std::vector<Row> rows_;
        Xyz::Vector2F origin_;
        float fps_ = 0.0f;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
        uint64_t last_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char** argv)
{
    try
    {
        argos::ArgumentParser parser;
        SdlApplication::add_command_line_options(parser);
        const auto args = parser.parse(argc, argv);
        SdlApplication app("NeoTable");
        app.read_command_line_options(args);
        app.run<NeoTableLoop>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
