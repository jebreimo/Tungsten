//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iosfwd>
#include <Xyz/Xyz.hpp>

#include "VertexArrayDataBuilder.hpp"

namespace Tungsten
{
    using PositionNormal = std::tuple<Xyz::Vector3F, Xyz::Vector3F>;
    using PositionNormalTexture = std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F>;

    void write(std::ostream& os, const PositionNormal& p);

    void write(std::ostream& os, const VertexArrayData<PositionNormal>& buffer);

    void write(std::ostream& os, const PositionNormalTexture& p);

    void write(std::ostream& os, const VertexArrayData<PositionNormalTexture>& buffer);

    template <typename... T>
    void add_rect_pn(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, T...>>& builder,
        const Xyz::Rectangle3F& rect)
    {
        const auto base_index = builder.max_index();
        const auto normal = rect.normal_vector();
        for (int i = 0; i < 4; ++i)
            builder.add_vertex({rect[i], normal, T()...});

        builder.add_indexes(base_index, base_index + 1, base_index + 2);
        builder.add_indexes(base_index, base_index + 2, base_index + 3);
    }

    template <typename... T>
    void add_cube_pn(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, T...>>& builder)
    {
        constexpr auto PI = Xyz::Constants<float>::PI;
        using O3F = Xyz::Orientation3F;
        constexpr auto size = Xyz::Vector2F(2, 2);
        add_rect_pn(builder, {{{-1, -1, 1}, O3F{0, 0, 0}}, size});
        add_rect_pn(builder, {{{-1, -1, -1}, O3F{0, 0, PI / 2}}, size});
        add_rect_pn(builder, {{{1, -1, -1}, O3F{PI / 2, 0, PI / 2}}, size});
        add_rect_pn(builder, {{{1, 1, -1}, O3F{PI, 0, PI / 2}}, size});
        add_rect_pn(builder, {{{-1, 1, -1}, O3F{-PI / 2, 0, PI / 2}}, size});
        add_rect_pn(builder, {{{-1, 1, -1}, O3F{0, 0, PI}}, size});
    }

    template <typename... T>
    void add_rect_pnt(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>>&
        builder,
        const Xyz::Rectangle3F& rect,
        const Xyz::Vector2F& uv_origin,
        const Xyz::Vector2F& uv_size)
    {
        const auto base_index = builder.max_index();
        add_rect_pn(builder, rect);
        std::get<2>(builder.vertex(base_index + 0)) = uv_origin;
        std::get<2>(builder.vertex(base_index + 1)) = uv_origin + Xyz::Vector2F(uv_size.x(), 0);
        std::get<2>(builder.vertex(base_index + 2)) = uv_origin + uv_size;
        std::get<2>(builder.vertex(base_index + 3)) = uv_origin + Xyz::Vector2F(0, uv_size.y());
    }

    template <typename... T>
    void add_cube_pnt(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>>&
        builder)
    {
        constexpr auto PI = Xyz::Constants<float>::PI;
        using O3F = Xyz::Orientation3F;
        constexpr auto size = Xyz::Vector2F(2, 2);
        constexpr auto uv_size = Xyz::Vector2F(1.f / 6, 1);
        add_rect_pnt(builder, {{{-1, -1, 1}, O3F{0, 0, 0}}, size}, {0, 0}, uv_size);
        add_rect_pnt(builder, {{{-1, -1, -1}, O3F{0, 0, PI / 2}}, size}, {1.f / 6, 0}, uv_size);
        add_rect_pnt(builder, {{{1, -1, -1}, O3F{PI / 2, 0, PI / 2}}, size}, {2.f / 6, 0}, uv_size);
        add_rect_pnt(builder, {{{1, 1, -1}, O3F{PI, 0, PI / 2}}, size}, {1.f / 2, 0}, uv_size);
        add_rect_pnt(builder, {{{-1, 1, -1}, O3F{-PI / 2, 0, PI / 2}}, size}, {4.f / 6, 0}, uv_size);
        add_rect_pnt(builder, {{{-1, 1, -1}, O3F{0, 0, PI}}, size}, {5.f / 6, 0}, uv_size);
    }
}
