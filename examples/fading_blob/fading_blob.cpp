//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-09.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// Neo drawing into somebody else's framebuffer. The trail is accumulated by
// SceneFader, which is hand-written GL because the scene graph has no notion
// of a render target — Renderer::render draws into whatever framebuffer is
// bound, which is exactly what makes the two compose:
//
//     fader.draw_previous_scene()   <- binds the accumulation buffer, fades it
//     ... the ordinary Neo pipeline ... <- draws the blob into that buffer
//     fader.render_scene()          <- puts the result on screen
//
// The blob's orbit is a hub node that rotates and a child offset along +x;
// resolve_transforms() composes the two, so nothing per-frame touches the
// blob's own transform or its vertices.

#include <cmath>
#include <cstring>
#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include <Tungsten/Gl/IOglWrapper.hpp>
#include "Resources.hpp"
#include "SceneFader.hpp"

namespace
{
    using namespace Tungsten;

    // Ids from FIRST_USER_SHADER_FAMILY up are the application's to allocate;
    // everything below it belongs to Neo's builtins.
    constexpr ShaderFamilyId BLOB_FAMILY = FIRST_USER_SHADER_FAMILY;

    // One 8-bit level subtracted per frame. See README.txt for why the fade is
    // subtractive rather than a multiplication by something just below 1.
    constexpr float FADE_STEP = 1.0f / 256.0f;

    constexpr float ORBIT_RADIUS = 0.5f;
    constexpr float BLOB_RADIUS = 0.2f;

    class FadingBlob : public EventLoop
    {
    public:
        explicit FadingBlob(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_),
              fader_(resources_, app.window_size()),
              window_size_(app.window_size())
        {
            register_blob_family();
            const auto shader = resources_.register_shader_variant({BLOB_FAMILY, 0});

            // No culling: this example never enabled it, and a flat quad has
            // no back face worth rejecting.
            PipelineDescriptor descriptor;
            descriptor.shader = shader;
            descriptor.layout = blob_layout();
            descriptor.raster.cull_enabled = false;
            const auto pipeline = resources_.register_pipeline(descriptor);

            const auto material = make_material(pipeline, {1, 1, 1, 1});
            const auto mesh = make_square_mesh();

            // The orbit is the hub's rotation and nothing else: the blob is a
            // child offset along +x, and the resolve pass composes the two
            // every frame (§2). The blob's own transform is set once, here.
            hub_ = scene_.add_node();
            blob_ = hub_.add_child();
            blob_.set_local_transform({
                .translation = {ORBIT_RADIUS, 0, 0},
                .scale = {BLOB_RADIUS, BLOB_RADIUS, 1}
            });
            blob_.add(RenderableComponent{
                .mesh = mesh,
                .material = material,
                .local_bounds = {{-1, -1, 0}, {1, 1, 0}}
            });

            // A 2D scene is nodes at z = 0 under an orthographic camera (§8),
            // placed in front of that plane looking down its own -z axis.
            camera_ = scene_.add_node();
            camera_.set_local_transform({.translation = {0, 0, 1}});
            camera_.add(CameraComponent{
                .mode = ProjectionMode::ORTHOGRAPHIC,
                .near_plane = 0.5f,
                .far_plane = 10.0f,
                .ortho_size = 1.0f,
                .aspect = app.viewport().aspect_ratio()
            });

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        void on_update() override
        {
            Transform hub;
            hub.rotation = z_rotation(
                float(SDL_GetTicks() - start_ticks_) * 0.001f);
            hub_.set_local_transform(hub);
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            // The window size is only known in pixels here;
            // SDL_EVENT_WINDOW_RESIZED reports screen coordinates, which
            // differ on high-DPI displays.
            const auto size = Xyz::vector_cast<int>(viewport.size);
            if (size != window_size_)
            {
                window_size_ = size;
                fader_.set_window_size(size);
            }
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            // Darkens last frame's contents into the accumulation buffer.
            fader_.draw_previous_scene(renderer_, FADE_STEP);

            // The accumulation pass loads rather than clears: the faded copy
            // underneath is the trail.
            scene_.resolve_transforms();
            auto& snapshots = snapshots_;
            builder_.build(scene_, camera_.id(), snapshots.back());
            snapshots.swap();
            renderer_.render(snapshots.front(),
                             fader_.accumulation_pass(viewport));

            fader_.render_scene(renderer_, viewport);

            // Single-threaded: the frame just drawn is complete (§11).
            resources_.collect_garbage(frame_);
            ++frame_;
            set_ogl_tracing_enabled(false);
            redraw();
        }

    private:
        void register_blob_family()
        {
            ShaderFamily family;
            family.vertex_source = BLOB_VERTEX;
            family.fragment_source = BLOB_FRAGMENT;
            family.required_attributes =
                semantic_bit(AttributeSemantic::POSITION);
            resources_.register_shader_family(BLOB_FAMILY, std::move(family));
        }

        MaterialRef make_material(PipelineRef pipeline,
                                  const Xyz::Vector4F& color)
        {
            const float values[4] = {color[0], color[1], color[2], color[3]};
            const auto bytes = as_bytes(std::span(values));
            Material material;
            material.pipeline = pipeline;
            material.parameter_data = {bytes.begin(), bytes.end()};
            return resources_.create_material(std::move(material));
        }

        // POSITION defaults to three components; the blob is flat, so it
        // carries two and the shader supplies z. Named by both the mesh and
        // the pipeline, which have to agree on the packing.
        VertexLayoutRef blob_layout()
        {
            return resources_.register_layout(
                VertexLayoutBuilder()
                    .add_attribute(AttributeSemantic::POSITION)
                    .set_component_count(2)
                    .build());
        }

        MeshRef make_square_mesh()
        {
            constexpr uint32_t VERTEX_COUNT = 4;
            constexpr uint32_t INDEX_COUNT = 6;
            constexpr float vertexes[2 * VERTEX_COUNT] = {
                -1, -1,
                1, -1,
                1, 1,
                -1, 1
            };
            uint16_t indexes[INDEX_COUNT] = {
                0, 1, 2,
                0, 2, 3
            };

            const auto layout = blob_layout();
            vbo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, 2 * sizeof(float), VERTEX_COUNT);
            ebo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, sizeof(uint16_t), INDEX_COUNT);

            const auto vertices = resources_.allocate(vbo_arena_, VERTEX_COUNT);
            const auto indices = resources_.allocate(ebo_arena_, INDEX_COUNT);

            // Rebase to absolute indices (§3): the portable path has no
            // baseVertex draw argument.
            for (auto& index : indexes)
                index = uint16_t(index + vertices.offset);

            resources_.upload(vertices, vertexes, sizeof(vertexes));
            resources_.upload(indices, indexes, sizeof(indexes));

            Mesh mesh;
            mesh.streams = {vertices};
            mesh.layout = layout;
            mesh.ebo = indices;
            return resources_.create_mesh(std::move(mesh));
        }

        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        SceneFader fader_;
        Size2I window_size_;
        BufferArenaRef vbo_arena_;
        BufferArenaRef ebo_arena_;
        NodeHandle hub_;
        NodeHandle blob_;
        NodeHandle camera_;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char* argv[])
{
    try
    {
        SdlApplication app("Fading blob");
        app.parse_command_line_options(argc, argv);
        set_ogl_tracing_enabled(true);
        app.run<FadingBlob>();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
