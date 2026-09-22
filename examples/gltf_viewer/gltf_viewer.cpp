//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// A minimal glTF viewer: load the file named on the command line, centre it,
// and turn it slowly under a two-light studio rig.
//
// The model is framed by its bounding *sphere* rather than its box. A sphere
// is what stays put while the model rotates, so the framing computed once at
// startup is still correct a quarter turn later; framing the box would make
// the model breathe in and out as its silhouette changed.
//
// Everything draws through the builtin Blinn-Phong family — see
// Tungsten/Import/GltfImport.hpp for how the file's metallic-roughness
// materials are approximated to reach it.

#include <cmath>
#include <iostream>
#include <string>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>
#include <Tungsten/Import/GltfImport.hpp>

namespace
{
    using namespace Tungsten;

    // Radians per second about +y. Slow enough to study the model, fast
    // enough that it is obviously moving.
    constexpr float ROTATION_SPEED = 0.3f;

    // How much wider than the model the frame is. A little air stops the
    // silhouette touching the window edge as it turns.
    constexpr float FRAMING_MARGIN = 1.15f;

    class GltfViewer : public EventLoop
    {
    public:
        GltfViewer(SdlApplication& app, std::string file_name)
            : EventLoop(app),
              renderer_(resources_)
        {
            register_builtin_shader_families(resources_);

            const auto import = import_gltf(file_name, resources_, scene_);
            for (const auto& warning : import.warnings)
                std::cerr << "Warning: " << warning << '\n';

            // The pivot turns; the model hangs under it shifted so that its
            // centre sits on the pivot. That is what makes it spin in place
            // rather than orbit.
            const auto centre = (import.bounds.min + import.bounds.max) / 2.0f;
            radius_ = Xyz::get_length(import.bounds.max - import.bounds.min) / 2;

            pivot_ = scene_.add_node();
            NodeHandle model(&scene_, import.root);
            model.reparent(pivot_);
            model.set_local_transform(Transform{.translation = -centre});

            camera_ = scene_.add_node();
            camera_.add(CameraComponent{.aspect = app.viewport().aspect_ratio()});
            place_camera();

            // A key and a dimmer fill from the opposite side, so that the
            // model stays readable through a full turn. Their intensities sum
            // to less than one: a glTF base color is an albedo, and lighting
            // it with more than it reflects blows the highlights out.
            add_light({-0.5f, 0.6f, 0.0f}, {1.0f, 0.97f, 0.9f}, 0.75f);
            add_light({0.3f, -2.4f, 0.0f}, {0.8f, 0.85f, 1.0f}, 0.25f);

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        void on_update() override
        {
            const auto t = float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            pivot_.set_local_transform(
                Transform{.rotation = euler_rotation(0, ROTATION_SPEED * t, 0)});
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.12f, 0.13f, 0.16f, 1.0f}}
            };

            // Through the handle, not a cached pointer: the component arrays
            // move their contents as they grow.
            auto& camera = camera_.get<CameraComponent>();
            if (camera.aspect != viewport.aspect_ratio())
            {
                camera.aspect = viewport.aspect_ratio();
                place_camera();
            }

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots_.back().ambient_light =
                srgb_to_linear(Xyz::Vector3F{0.20f, 0.21f, 0.25f});
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        /**
         * Backs the camera off far enough that a sphere of radius_ at the
         * origin fits the viewport, and sets the depth range to straddle it.
         */
        void place_camera()
        {
            auto& camera = camera_.get<CameraComponent>();

            // The vertical field of view is the given one; the horizontal
            // follows from the aspect ratio, and is the tighter of the two on
            // a window taller than it is wide.
            const auto half_v = camera.fov / 2;
            const auto half_h = std::atan(std::tan(half_v) * camera.aspect);
            const auto half_angle = std::min(half_v, half_h);

            const auto distance = FRAMING_MARGIN * radius_ / std::sin(half_angle);
            camera.near_plane = std::max(distance - radius_, distance * 1e-3f);
            camera.far_plane = distance + 2 * radius_;
            camera_.set_local_transform(
                Transform{.translation = {0, 0, distance}});
        }

        /**
         * Adds a directional light aimed by rotating its node: a light shines
         * along its node's -z axis.
         */
        void add_light(const Xyz::Vector3F& euler,
                       const Xyz::Vector3F& color, float intensity)
        {
            auto node = scene_.add_node();
            node.set_local_transform(Transform{
                .rotation = euler_rotation(euler[0], euler[1], euler[2])
            });
            node.add(LightComponent{
                .type = LightType::DIRECTIONAL,
                .color = color,
                .intensity = intensity
            });
        }

        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        NodeHandle pivot_;
        NodeHandle camera_;
        float radius_ = 1.0f;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char** argv)
{
    try
    {
        argos::ArgumentParser parser(argv[0]);
        parser.about("Displays a glTF model, centred and slowly rotating.")
            .add(argos::Argument("FILE")
                .help("The glTF or GLB file to display."));
        Tungsten::SdlApplication::add_command_line_options(parser);
        const auto args = parser.parse(argc, argv);

        Tungsten::SdlApplication app("GltfViewer");
        app.read_command_line_options(args);
        app.run<GltfViewer>(args.value("FILE").as_string());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
