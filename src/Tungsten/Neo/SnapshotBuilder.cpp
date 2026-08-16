//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/SnapshotBuilder.hpp"

#include <algorithm>
#include <Xyz/InvertMatrix.hpp>

#include "Tungsten/Neo/CameraComponent.hpp"
#include "Tungsten/Neo/LightComponent.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/Scene.hpp"

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
    }

    SnapshotBuilder::SnapshotBuilder(ResourceManager& resources)
        : resources_(resources)
    {}

    void SnapshotBuilder::build(const Scene& scene, NodeId camera_node,
                                RenderSnapshot& out)
    {
        out.clear();

        const auto& camera = scene.get_component<CameraComponent>(camera_node);
        const Xyz::Matrix4F& camera_world = scene.world_matrix(camera_node);

        view_ = CameraComponent::get_view_matrix(camera_world);
        far_plane_ = camera.far_plane;
        const auto projection = camera.get_projection_matrix();
        frustum_planes_ = extract_frustum_planes(projection * view_);

        out.view_matrix = view_;
        out.projection_matrix = projection;
        out.camera_position = translation_of(camera_world);

        const size_t count = prepare_bounds(scene);
        cull(count);
        extract_renderables(scene, count, out);
        extract_lights(scene, out);
    }

    size_t SnapshotBuilder::prepare_bounds(const Scene& scene)
    {
        const auto& store = scene.components<RenderableComponent>();
        const size_t count = store.items.size();

        min_x_.resize(count);
        min_y_.resize(count);
        min_z_.resize(count);
        max_x_.resize(count);
        max_y_.resize(count);
        max_z_.resize(count);
        drawable_.resize(count);
        cullable_.resize(count);
        visible_.assign(count, 1);

        for (size_t i = 0; i < count; ++i)
        {
            const RenderableComponent& renderable = store.items[i];
            const bool drawable = renderable.visible
                                  && bool(renderable.mesh)
                                  && bool(renderable.material);
            drawable_[i] = uint8_t(drawable);
            cullable_[i] = uint8_t(drawable && bool(renderable.local_bounds));

            if (!cullable_[i])
            {
                // Left at zero rather than skipped: the cull loop reads every
                // lane unconditionally, and these lanes are discarded after.
                min_x_[i] = min_y_[i] = min_z_[i] = 0.0f;
                max_x_[i] = max_y_[i] = max_z_[i] = 0.0f;
                continue;
            }

            const Xyz::BBox3F box = Xyz::transform_bbox_no_w(
                renderable.local_bounds,
                scene.world_matrix(store.owners[i]));
            min_x_[i] = box.min[0];
            min_y_[i] = box.min[1];
            min_z_[i] = box.min[2];
            max_x_[i] = box.max[0];
            max_y_[i] = box.max[1];
            max_z_[i] = box.max[2];
        }

        return count;
    }

    void SnapshotBuilder::cull(size_t count)
    {
        for (const auto& plane : frustum_planes_)
        {
            // The positive-vertex test: only the corner farthest along the
            // plane normal can keep the box inside, and which corner that is
            // depends on the plane alone — so the choice is made once here
            // rather than per item.
            const float* xs = plane[0] >= 0 ? max_x_.data() : min_x_.data();
            const float* ys = plane[1] >= 0 ? max_y_.data() : min_y_.data();
            const float* zs = plane[2] >= 0 ? max_z_.data() : min_z_.data();
            const float a = plane[0];
            const float b = plane[1];
            const float c = plane[2];
            const float d = plane[3];

            for (size_t i = 0; i < count; ++i)
                visible_[i] &= uint8_t(a * xs[i] + b * ys[i] + c * zs[i] + d
                                       >= 0.0f);
        }
    }

    void SnapshotBuilder::extract_renderables(const Scene& scene, size_t count,
                                              RenderSnapshot& out)
    {
        const auto& store = scene.components<RenderableComponent>();

        for (size_t i = 0; i < count; ++i)
        {
            if (!drawable_[i] || (cullable_[i] && !visible_[i]))
                continue;

            const RenderableComponent& renderable = store.items[i];
            const Xyz::Matrix4F& world =
                scene.world_matrix(store.owners[i]);
            const Material& material =
                resources_.get_material(renderable.material);

            // The view-space distance to the item's origin, for depth
            // ordering.
            const Xyz::Vector3F position = translation_of(world);
            const float view_z = view_[2, 0] * position[0]
                                 + view_[2, 1] * position[1]
                                 + view_[2, 2] * position[2]
                                 + view_[2, 3];
            // The camera looks along -z, so distance in front of it is -view_z.
            const float depth = std::clamp(-view_z / far_plane_, 0.0f, 1.0f);

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

    void SnapshotBuilder::extract_lights(const Scene& scene,
                                         RenderSnapshot& out)
    {
        const auto& store = scene.components<LightComponent>();

        for (size_t i = 0; i < store.items.size(); ++i)
        {
            const LightComponent& light = store.items[i];
            const Xyz::Matrix4F& world = scene.world_matrix(store.owners[i]);

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

    uint64_t SnapshotBuilder::compute_sort_key(uint32_t layer,
                                               uint32_t shader_index,
                                               uint32_t material_index,
                                               uint32_t mesh_index,
                                               float normalized_depth,
                                               bool transparent)
    {
        // The indices are batching hints, not identities: truncating them can
        // only cost a missed batch, never a wrong draw.
        const auto layer8 = static_cast<uint64_t>(layer & 0xFF);
        const auto shader12 = static_cast<uint64_t>(shader_index & 0xFFF);
        const auto material16 = static_cast<uint64_t>(material_index & 0xFFFF);
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

    Xyz::Matrix3F SnapshotBuilder::compute_normal_matrix(
        const Xyz::Matrix4F& world)
    {
        Xyz::Matrix3F upper;
        for (size_t row = 0; row < 3; ++row)
        {
            for (size_t col = 0; col < 3; ++col)
                upper[row, col] = world[row, col];
        }
        return Xyz::transpose(Xyz::invert(upper));
    }
} // Tungsten