//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-05.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// Text anchoring, drawn through Neo's scene graph: the same block of text
// pinned to each corner of the window and to its centre, each with the anchor
// that keeps it inside the window as it grows.
#include <array>
#include <chrono>
#include <iostream>
#include <thread>
#include <Tungsten/Tungsten.hpp>

namespace
{
    using namespace Tungsten;

    const Xyz::Vector4F RED = {1.f, 0.f, 0.f, 1.f};
    const Xyz::Vector4F GREEN = {0.f, 1.f, 0.f, 1.f};
    const Xyz::Vector4F BLUE = {0.f, 0.f, 1.f, 1.f};
    const Xyz::Vector4F BLACK = {0.f, 0.f, 0.f, 1.f};
    const Xyz::Vector4F WHITE = {1.f, 1.f, 1.f, 1.f};

    // Where a block sits, in viewport-normalized coordinates: (0, 0) is the
    // bottom-left corner, (1, 1) the top-right.
    struct Corner
    {
        Xyz::Vector2F position;
        Xyz::Vector4F color;
    };

    const std::array<Corner, 5> CORNERS = {{
        {{0.0f, 0.0f}, RED},
        {{0.0f, 1.0f}, GREEN},
        {{1.0f, 0.0f}, BLUE},
        {{1.0f, 1.0f}, BLACK},
        {{0.5f, 0.5f}, WHITE}
    }};

    HorizontalAlignment get_horizontal_alignment(float x)
    {
        if (x == 0)
            return HorizontalAlignment::LEFT;
        if (x == 1)
            return HorizontalAlignment::RIGHT;
        return HorizontalAlignment::CENTER;
    }

    HorizontalAnchor get_horizontal_anchor(float x)
    {
        if (x == 0)
            return HorizontalAnchor::LEFT;
        if (x == 1)
            return HorizontalAnchor::RIGHT;
        return HorizontalAnchor::CENTER;
    }

    VerticalAnchor get_vertical_anchor(float y)
    {
        if (y == 0)
            return VerticalAnchor::BOTTOM;
        if (y == 1)
            return VerticalAnchor::TOP;
        return VerticalAnchor::CENTER;
    }

    Transform at(float x, float y, float z = 0.0f)
    {
        Transform transform;
        transform.translation = {x, y, z};
        return transform;
    }

    class ShowText : public EventLoop
    {
    public:
        explicit ShowText(SdlApplication& app)
            : EventLoop(app),
              text_system_(resources_),
              renderer_(resources_)
        {
            const std::shared_ptr<const Font> font =
                font_manager_.default_font();

            for (const auto& [position, color] : CORNERS)
            {
                // One style per block: they differ in colour and in which
                // point of the text lands on the node's origin. Styles are
                // immutable, so each is made once and never touched again.
                auto node = scene_.add_node();
                node.add(TextComponent{
                    .style = make_text_style({
                        .font = font,
                        .color = color,
                        .horizontal_alignment =
                            get_horizontal_alignment(position.x()),
                        .horizontal_anchor =
                            get_horizontal_anchor(position.x()),
                        .vertical_anchor = get_vertical_anchor(position.y())
                    })
                });
                nodes_.push_back(node);
            }

            // A 2D scene is nodes at z = 0 seen by an orthographic camera. The
            // camera sits in front of that plane looking down its node's -z
            // axis.
            camera_ = scene_.add_node();
            camera_.set_local_transform(at(0, 0, 100));
            camera_.add(CameraComponent{
                .mode = ProjectionMode::ORTHOGRAPHIC,
                .near_plane = 1.0f,
                .far_plane = 200.0f,
                .aspect = app.viewport().aspect_ratio()
            });
        }

        void on_update() override
        {
            const auto current_second = SDL_GetTicks() / 1000;
            if (current_second != second_)
            {
                second_ = current_second;
                const auto text = "Jan Erik Breimo\nNatasha Barrett\nTime: "
                                  + std::to_string(second_);
                // One component is one text run however many lines it has, so
                // each of these is a single mesh and a single draw.
                for (auto& node : nodes_)
                    node.get<TextComponent>().text = text;
                redraw();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            set_viewport(viewport);
            set_clear_color(0.4f, 0.6f, 0.8f, 1.0f);
            clear(ClearBits::COLOR_DEPTH);

            // One world unit per pixel: the orthographic half-height is half
            // the viewport's, so the visible volume is exactly the window in
            // pixels, with the origin at its centre.
            auto& camera = camera_.get<CameraComponent>();
            camera.ortho_size = viewport.size[1] / 2.0f;
            camera.aspect = viewport.aspect_ratio();

            // Follow the window's corners. This runs every draw and re-uploads
            // nothing — the position reaches the shader through the per-draw
            // UBO, never through the vertex data.
            for (size_t i = 0; i < nodes_.size(); ++i)
            {
                const auto centre_offset =
                    (CORNERS[i].position - Xyz::Vector2F(0.5f, 0.5f))
                    * viewport.size;
                nodes_[i].set_local_transform(
                    at(centre_offset.x(), centre_offset.y()));
            }

            resources_.begin_frame(frame_);

            // Before the resolve, so the meshes and bounds it publishes are in
            // place for the extraction pass that follows.
            text_system_.update(scene_);
            scene_.resolve_transforms();

            auto& snapshots = snapshots_;
            builder_.build(scene_, camera_.id(), snapshots.back());
            snapshots.swap();
            renderer_.render(snapshots.front());

            // Single-threaded: the frame just drawn is complete.
            resources_.collect_garbage(frame_);
            ++frame_;

            set_ogl_tracing_enabled(false);
        }

    private:
        ResourceManager resources_;
        FontManager font_manager_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        TextSystem text_system_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;

        std::vector<NodeHandle> nodes_;
        NodeHandle camera_;
        uint64_t frame_ = 0;
        uint32_t second_ = UINT32_MAX;
    };
}

int main(int argc, char* argv[])
{
    try
    {
        SdlApplication app("ShowText");
        app.parse_command_line_options(argc, argv);
        set_ogl_tracing_enabled(true);
        app.run<ShowText>();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
