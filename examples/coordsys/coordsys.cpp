//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-06.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// An infinite grid on the world's xz plane, drawn with the "pristine grid"
// shader from Ben Golus' article The Best Darn Grid Shader (Yet).
//
// The grid is a single square that the vertex shader keeps centred below the
// camera, and which is big enough to reach past the far plane in every
// direction. All the work is in grid-frag.glsl.
//
// Controls: drag with the left mouse button to orbit, with the right button
// to move across the grid, and use the wheel to zoom.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <span>

#include "Tungsten/Tungsten.hpp"
#include "Resources.hpp"

namespace
{
    using namespace Tungsten;

    constexpr ShaderFamilyId GRID_FAMILY = FIRST_USER_SHADER_FAMILY;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;

    // The width of the grid lines as a fraction of a cell.
    constexpr float LINE_WIDTH = 0.02f;
    // The number of cells per world unit.
    constexpr float GRID_SCALE = 1.0f;

    constexpr float ORBIT_SPEED = 0.005f;
    constexpr float PAN_SPEED = 0.002f;
    constexpr float ZOOM_STEP = 1.1f;
    constexpr float MIN_DISTANCE = 0.5f;
    constexpr float MAX_DISTANCE = 500.0f;
    constexpr float MAX_PITCH = 1.55f;

    class Coordsys : public EventLoop
    {
    public:
        explicit Coordsys(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            register_grid_family();
            const auto shader = resources_.register_shader_variant({GRID_FAMILY, 0});

            // No culling: the grid is as visible from below as from above.
            PipelineDescriptor descriptor;
            descriptor.shader = shader;
            descriptor.layout = grid_layout();
            descriptor.raster.cull_enabled = false;
            const auto pipeline = resources_.register_pipeline(descriptor);

            // No local bounds: the square follows the camera, so it must
            // never be culled.
            grid_ = scene_.add_node();
            grid_.add(RenderableComponent{
                .mesh = make_square_mesh(),
                .material = make_material(pipeline)
            });

            camera_ = scene_.add_node();
            camera_.add(CameraComponent{
                .near_plane = NEAR_PLANE,
                .far_plane = FAR_PLANE,
                .aspect = app.viewport().aspect_ratio()
            });
            place_camera();

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        bool on_event(const SDL_Event& event) override
        {
            switch (event.type)
            {
            case SDL_EVENT_MOUSE_MOTION:
                if (event.motion.state & SDL_BUTTON_LMASK)
                    orbit(event.motion.xrel, event.motion.yrel);
                else if (event.motion.state & SDL_BUTTON_RMASK)
                    pan(event.motion.xrel, event.motion.yrel);
                else
                    return false;
                return true;
            case SDL_EVENT_MOUSE_WHEEL:
                distance_ = std::clamp(
                    distance_ * std::pow(ZOOM_STEP, -event.wheel.y),
                    MIN_DISTANCE, MAX_DISTANCE);
                place_camera();
                return true;
            default:
                return false;
            }
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.45f, 0.55f, 0.65f, 1.0f}}
            };
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        void register_grid_family()
        {
            ShaderFamily family;
            family.vertex_source = GRID_VERTEX;
            family.fragment_source = GRID_FRAGMENT;
            family.required_attributes =
                semantic_bit(AttributeSemantic::POSITION);
            resources_.register_shader_family(GRID_FAMILY, std::move(family));
        }

        MaterialRef make_material(PipelineRef pipeline)
        {
            // The shader's MaterialBlock: three vec4s. Colours in a uniform
            // block are linear; these are picked by eye, hence sRGB.
            const auto lines = srgb_to_linear(
                Xyz::Vector4F{0.85f, 0.85f, 0.85f, 1.0f});
            const auto base = srgb_to_linear(
                Xyz::Vector4F{0.16f, 0.17f, 0.19f, 1.0f});
            // The square has to cover everything in front of the far plane
            // wherever the camera looks, and the corners of the far plane are
            // further away than FAR_PLANE.
            const float values[12] = {
                lines[0], lines[1], lines[2], lines[3],
                base[0], base[1], base[2], base[3],
                LINE_WIDTH, LINE_WIDTH, GRID_SCALE, 2 * FAR_PLANE
            };
            const auto bytes = std::as_bytes(std::span(values));
            Material material;
            material.pipeline = pipeline;
            material.parameter_data = {bytes.begin(), bytes.end()};
            return resources_.create_material(std::move(material));
        }

        // The corners are two-dimensional as the shader places them in the
        // xz plane. Named by both the mesh and the pipeline, which have to
        // agree on the packing.
        VertexLayoutRef grid_layout()
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

            vbo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, 2 * sizeof(float), VERTEX_COUNT);
            ebo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, sizeof(uint16_t), INDEX_COUNT);

            const auto vertices = resources_.allocate(vbo_arena_, VERTEX_COUNT);
            const auto indices = resources_.allocate(ebo_arena_, INDEX_COUNT);

            // Rebase to absolute indices: the portable path has no
            // baseVertex draw argument.
            for (auto& index : indexes)
                index = uint16_t(index + vertices.offset);

            resources_.upload(vertices, vertexes, sizeof(vertexes));
            resources_.upload(indices, indexes, sizeof(indexes));

            Mesh mesh;
            mesh.streams = {vertices};
            mesh.layout = grid_layout();
            mesh.ebo = indices;
            return resources_.create_mesh(std::move(mesh));
        }

        void orbit(float dx, float dy)
        {
            yaw_ -= dx * ORBIT_SPEED;
            pitch_ = std::clamp(pitch_ + dy * ORBIT_SPEED,
                                -MAX_PITCH, MAX_PITCH);
            place_camera();
        }

        // Moves the target across the grid such that the grid appears to
        // follow the mouse.
        void pan(float dx, float dy)
        {
            const auto step = PAN_SPEED * distance_;
            const Xyz::Vector3F right = {std::cos(yaw_), 0, -std::sin(yaw_)};
            const Xyz::Vector3F forward = {-std::sin(yaw_), 0, -std::cos(yaw_)};
            target_ = target_ - right * (dx * step) + forward * (dy * step);
            place_camera();
        }

        // Places the camera on a sphere around the target, looking at it.
        void place_camera()
        {
            const Xyz::Vector3F offset = {
                std::cos(pitch_) * std::sin(yaw_),
                std::sin(pitch_),
                std::cos(pitch_) * std::cos(yaw_)
            };
            const auto position = target_ + offset * distance_;
            camera_.set_local_transform({
                .translation = position,
                .rotation = look_at_rotation(position, target_)
            });
        }

        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        BufferArenaRef vbo_arena_;
        BufferArenaRef ebo_arena_;
        NodeHandle grid_;
        NodeHandle camera_;
        Xyz::Vector3F target_ = {0, 0, 0};
        float yaw_ = 0.5f;
        float pitch_ = 0.35f;
        float distance_ = 8.0f;
        uint64_t frame_ = 0;
    };
}

int main(int argc, char* argv[])
{
    try
    {
        SdlApplication app("Coordsys");
        app.parse_command_line_options(argc, argv);
        app.run<Coordsys>();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
