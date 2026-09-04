//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Rendering/SnapshotBuilder.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>
#include <Xyz/InvertMatrix.hpp>
#include <Xyz/Matrix.hpp>
#include <Xyz/Vector.hpp>

#include "Tungsten/SceneGraph/CameraComponent.hpp"
#include "Tungsten/SceneGraph/LightComponent.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"

namespace Tungsten
{
    namespace
    {
        Xyz::Vector3F translation_of(const Xyz::Matrix4F& m)
        {
            return {m[0, 3], m[1, 3], m[2, 3]};
        }

        // Gribb–Hartmann plane extraction: with column vectors and the GL
        // clip volume (-w <= x,y,z <= w), each frustum plane is a sum or
        // difference of rows of the view-projection matrix. The planes'
        // normals point into the frustum; they are not normalized, which is
        // fine for the sign-only test below.
        std::array<Xyz::Vector4F, 6>
        extract_frustum_planes(const Xyz::Matrix4F& view_projection)
        {
            const auto row = [&](unsigned r)
            {
                return Xyz::get_row(view_projection, r);
            };
            return {
                row(3) + row(0), row(3) - row(0),
                row(3) + row(1), row(3) - row(1),
                row(3) + row(2), row(3) - row(2)
            };
        }

        // The inverse transpose of the world matrix's upper 3x3, which keeps
        // normals perpendicular under non-uniform scale.
        Xyz::Matrix3F compute_normal_matrix(const Xyz::Matrix4F& world)
        {
            Xyz::Matrix3F upper;
            for (size_t row = 0; row < 3; ++row)
            {
                for (size_t col = 0; col < 3; ++col)
                    upper[row, col] = world[row, col];
            }
            return Xyz::transpose(Xyz::invert(upper));
        }

        // Packs the draw order into one integer, ascending (§8). Opaque:
        // layer, shader, material, mesh, then depth — batching state changes
        // first. Transparent: layer, then *reversed* depth — back-to-front
        // for correct blending — then shader and material.
        uint64_t compute_sort_key(uint32_t layer, uint32_t shader_index,
                                  uint32_t material_index, uint32_t mesh_index,
                                  float normalized_depth, bool transparent)
        {
            // The indices are batching hints, not identities: truncating them
            // can only cost a missed batch, never a wrong draw.
            const auto layer8 = static_cast<uint64_t>(layer & 0xFF);
            const auto shader12 = static_cast<uint64_t>(shader_index & 0xFFF);
            const auto material16 =
                static_cast<uint64_t>(material_index & 0xFFFF);
            const auto mesh12 = static_cast<uint64_t>(mesh_index & 0xFFF);

            if (transparent)
            {
                // layer(8) | reversed depth(24) | shader(12) | material(16) |
                // spare(4). Reversing the depth makes an ascending sort draw
                // back-to-front.
                const auto depth24 = static_cast<uint64_t>(
                    normalized_depth * float{0xFFFFFF});
                return layer8 << 56
                       | (0xFFFFFF - depth24) << 32
                       | shader12 << 20
                       | material16 << 4;
            }

            // layer(8) | shader(12) | material(16) | mesh(12) | depth(16):
            // state-change batching first, front-to-back within equal state.
            const auto depth16 = static_cast<uint64_t>(
                normalized_depth * float{0xFFFF});
            return layer8 << 56
                   | shader12 << 44
                   | material16 << 28
                   | mesh12 << 16
                   | depth16;
        }

        // Extracts lights from the scene into the RenderSnapshot. Reads no
        // builder state, so it is a plain function over the two arguments.
        void extract_lights(const Scene& scene, RenderSnapshot& out)
        {
            const auto& store = scene.components<LightComponent>();

            for (size_t i = 0; i < store.items.size(); ++i)
            {
                const LightComponent& light = store.items[i];
                const Xyz::Matrix4F& world =
                    scene.world_matrix(store.owners[i]);

                LightData data;
                data.set_type(light.type);
                data.set_position(translation_of(world));
                // The light shines along its node's -z axis (the camera
                // convention): the negated third column of the world matrix,
                // normalized to strip any scale.
                data.set_direction(Xyz::normalize(
                    Xyz::Vector3F{-world[0, 2], -world[1, 2], -world[2, 2]}));
                data.set_color(light.color);
                data.set_intensity(light.intensity);
                data.set_range(light.range);
                data.set_inner_cone_angle(light.inner_cone_angle);
                data.set_outer_cone_angle(light.outer_cone_angle);
                out.lights.push_back(data);
            }
        }
    }

    struct SnapshotBuilder::Members
    {
        explicit Members(ResourceManager& resources)
            : resources(resources)
        {}

        void build(const Scene& scene, NodeId camera_node, RenderSnapshot& out);

        /**
         * Fills the SoA bounds scratch and the per-item flags for every
         * renderable in the scene. The scratch arrays are sized to the scene's
         * renderable count, which is what the two passes below iterate over.
         */
        void prepare_bounds(const Scene& scene);

        /**
         * Clears visible for every item whose bounds fall entirely outside
         * the frustum. Planes outer, items inner: the sign of each plane's
         * normal picks a bounds array once per plane instead of once per item,
         * which leaves the inner loop free of branches.
         */
        void cull();

        /**
         * Extracts renderables from the scene into the RenderSnapshot.
         */
        void extract_renderables(const Scene& scene, RenderSnapshot& out);

        ResourceManager& resources;

        // Per-build state, set up by build() and read during extraction.
        std::array<Xyz::Vector4F, 6> frustum_planes;
        Xyz::Matrix4F view;
        float far_plane = 1.0f;

        // Scratch, kept across frames so a steady-state build does not
        // allocate. One array per bounds component, parallel to the scene's
        // renderable array.
        std::vector<float> min_x, min_y, min_z;
        std::vector<float> max_x, max_y, max_z;

        /**
         * Has a mesh, a material, and is visible at all.
         */
        std::vector<uint8_t> drawable;
        /**
         * Drawable and has bounds — empty bounds mean "unknown", which is never culled.
         */
        std::vector<uint8_t> cullable;
        /**
         * Survived the frustum test. Meaningless where !cullable.
         */
        std::vector<uint8_t> visible;
    };

    void SnapshotBuilder::Members::build(const Scene& scene,
                                         NodeId camera_node,
                                         RenderSnapshot& out)
    {
        out.clear();

        const auto& camera = scene.get_component<CameraComponent>(camera_node);
        const Xyz::Matrix4F& camera_world = scene.world_matrix(camera_node);

        view = CameraComponent::get_view_matrix(camera_world);
        far_plane = camera.far_plane;
        const auto projection = camera.get_projection_matrix();
        frustum_planes = extract_frustum_planes(projection * view);

        out.view_matrix = view;
        out.projection_matrix = projection;
        out.camera_position = translation_of(camera_world);

        prepare_bounds(scene);
        cull();
        extract_renderables(scene, out);
        extract_lights(scene, out);
    }

    void SnapshotBuilder::Members::prepare_bounds(const Scene& scene)
    {
        const auto& store = scene.components<RenderableComponent>();
        const size_t count = store.items.size();

        min_x.resize(count);
        min_y.resize(count);
        min_z.resize(count);
        max_x.resize(count);
        max_y.resize(count);
        max_z.resize(count);
        drawable.resize(count);
        cullable.resize(count);
        visible.assign(count, 1);

        for (size_t i = 0; i < count; ++i)
        {
            const RenderableComponent& renderable = store.items[i];
            const bool is_drawable = renderable.visible
                                     && bool(renderable.mesh)
                                     && bool(renderable.material);
            drawable[i] = uint8_t(is_drawable);
            cullable[i] = uint8_t(is_drawable
                                  && bool(renderable.local_bounds));

            if (!cullable[i])
            {
                // Left at zero rather than skipped: the cull loop reads every
                // lane unconditionally, and these lanes are discarded after.
                min_x[i] = min_y[i] = min_z[i] = 0.0f;
                max_x[i] = max_y[i] = max_z[i] = 0.0f;
                continue;
            }

            const Xyz::BBox3F box = Xyz::transform_bbox_no_w(
                renderable.local_bounds,
                scene.world_matrix(store.owners[i]));
            min_x[i] = box.min[0];
            min_y[i] = box.min[1];
            min_z[i] = box.min[2];
            max_x[i] = box.max[0];
            max_y[i] = box.max[1];
            max_z[i] = box.max[2];
        }
    }

    void SnapshotBuilder::Members::cull()
    {
        const size_t count = visible.size();
        for (const auto& plane : frustum_planes)
        {
            // The positive-vertex test: only the corner farthest along the
            // plane normal can keep the box inside, and which corner that is
            // depends on the plane alone — so the choice is made once here
            // rather than per item.
            const float* xs = plane[0] >= 0 ? max_x.data() : min_x.data();
            const float* ys = plane[1] >= 0 ? max_y.data() : min_y.data();
            const float* zs = plane[2] >= 0 ? max_z.data() : min_z.data();
            const float a = plane[0];
            const float b = plane[1];
            const float c = plane[2];
            const float d = plane[3];

            for (size_t i = 0; i < count; ++i)
                visible[i] &= uint8_t(a * xs[i] + b * ys[i] + c * zs[i] + d
                                      >= 0.0f);
        }
    }

    void SnapshotBuilder::Members::extract_renderables(const Scene& scene,
                                                       RenderSnapshot& out)
    {
        const auto& store = scene.components<RenderableComponent>();

        for (size_t i = 0; i < store.items.size(); ++i)
        {
            if (!drawable[i] || (cullable[i] && !visible[i]))
                continue;

            const RenderableComponent& renderable = store.items[i];
            const Xyz::Matrix4F& world =
                scene.world_matrix(store.owners[i]);
            const Material& material =
                resources.get_material(renderable.material);

            // The view-space distance to the item's origin, for depth
            // ordering.
            const Xyz::Vector3F position = translation_of(world);
            const float view_z = view[2, 0] * position[0]
                                 + view[2, 1] * position[1]
                                 + view[2, 2] * position[2]
                                 + view[2, 3];
            // The camera looks along -z, so distance in front of it is -view_z.
            const float depth = std::clamp(-view_z / far_plane, 0.0f, 1.0f);

            RenderItem item;
            item.set_world_transform(world);
            item.set_normal_matrix(compute_normal_matrix(world));
            item.set_mesh(renderable.mesh);
            item.set_material(renderable.material);
            item.set_sort_key(compute_sort_key(renderable.render_layer,
                                               material.shader.index,
                                               renderable.material.index,
                                               renderable.mesh.index,
                                               depth,
                                               material.transparent));

            auto& items = material.transparent ? out.transparent_items
                                               : out.opaque_items;
            items.push_back(item);
        }
    }

    SnapshotBuilder::SnapshotBuilder(ResourceManager& resources)
        : members_(std::make_unique<Members>(resources))
    {
    }

    SnapshotBuilder::~SnapshotBuilder() = default;

    void SnapshotBuilder::build(const Scene& scene, NodeId camera_node,
                                RenderSnapshot& out)
    {
        members_->build(scene, camera_node, out);
    }
} // Tungsten
