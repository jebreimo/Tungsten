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

        // True if the box is entirely on the outside of the plane, using the
        // positive-vertex test: only the corner farthest along the plane
        // normal needs checking.
        bool outside_plane(const Xyz::Vector4F& plane, const AABB& box)
        {
            const Xyz::Vector3F corner = {
                plane[0] >= 0 ? box.max[0] : box.min[0],
                plane[1] >= 0 ? box.max[1] : box.min[1],
                plane[2] >= 0 ? box.max[2] : box.min[2]
            };
            return plane[0] * corner[0] + plane[1] * corner[1]
                   + plane[2] * corner[2] + plane[3] < 0;
        }
    }

    SnapshotBuilder::SnapshotBuilder(ResourceManager& resources)
        : resources_(resources)
    {}

    void SnapshotBuilder::build(const Scene& scene,
                                const CameraComponent& camera,
                                RenderSnapshot& out)
    {
        out.clear();

        view_ = camera.get_view_matrix();
        far_plane_ = camera.far_plane;
        const auto projection = camera.get_projection_matrix();
        frustum_planes_ = extract_frustum_planes(projection * view_);

        out.view_matrix = view_;
        out.projection_matrix = projection;
        if (const Node* node = camera.owner())
            out.camera_position = translation_of(node->world_matrix());

        for (const auto& root : scene.roots())
            extract_node(*root, out);
    }

    void SnapshotBuilder::extract_node(const Node& node, RenderSnapshot& out) // NOLINT(*-no-recursion)
    {
        for (const auto& component : node.components())
        {
            if (const auto* renderable =
                    dynamic_cast<const RenderableComponent*>(component.get()))
            {
                extract_renderable(node, *renderable, out);
            }
            else if (const auto* light =
                    dynamic_cast<const LightComponent*>(component.get()))
            {
                out.lights.push_back(extract_light(node, *light));
            }
        }
        for (const auto& child : node.children())
            extract_node(*child, out);
    }

    void SnapshotBuilder::extract_renderable(
        const Node& node,
        const RenderableComponent& renderable,
        RenderSnapshot& out)
    {
        if (!renderable.visible || !renderable.mesh || !renderable.material)
            return;

        const Xyz::Matrix4F& world = node.world_matrix();
        // The item is culled against its *own* world-space bounds; the
        // aggregate Node::world_bounds is reserved for a hierarchical
        // subtree test later.
        if (cull(transformed(renderable.local_bounds, world)))
            return;

        const Material& material = resources_.get_material(renderable.material);

        // The view-space distance to the item's origin, for depth ordering.
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

    LightData SnapshotBuilder::extract_light(const Node& node,
                                             const LightComponent& light)
    {
        const Xyz::Matrix4F& world = node.world_matrix();

        LightData data;
        data.set_type(light.type);
        data.set_position(translation_of(world));
        // The light shines along its node's -z axis (the camera convention):
        // the negated third column of the world matrix, normalized to strip
        // any scale.
        data.set_direction(Xyz::normalize(
            Xyz::Vector3F{-world[0, 2], -world[1, 2], -world[2, 2]}));
        data.set_color(light.color);
        data.set_intensity(light.intensity);
        data.set_range(light.range);
        data.set_inner_cone_angle(light.inner_cone_angle);
        data.set_outer_cone_angle(light.outer_cone_angle);
        return data;
    }

    bool SnapshotBuilder::cull(const AABB& world_bounds) const
    {
        if (world_bounds.is_empty())
            return false;
        for (const auto& plane : frustum_planes_)
        {
            if (outside_plane(plane, world_bounds))
                return true;
        }
        return false;
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
