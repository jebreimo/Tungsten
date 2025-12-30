//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <ostream>
#include <span>
#include <Xyz/Xyz.hpp>

#include "VertexArrayDataBuilder.hpp"

namespace Tungsten
{
    using PositionNormal = std::tuple<Xyz::Vector3F, Xyz::Vector3F>;
    using PositionNormalTexture = std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F>;

    void write_triangles(std::ostream& os,
                         std::span<const uint16_t> indices,
                         bool indexes = true);

    template <typename... T>
    void write_pn(std::ostream& os,
                  const std::tuple<Xyz::Vector3F, Xyz::Vector3F, T...>& p)
    {
        os << std::get<0>(p) << " " << std::get<1>(p) << '\n';
    }

    template <typename... T>
    void write_pn(std::ostream& os,
                  std::span<const std::tuple<Xyz::Vector3F, Xyz::Vector3F, T...>> p,
                  bool indexes = true)
    {
        for (size_t i = 0; i < p.size(); ++i)
        {
            if (indexes)
                os << i << ": ";
            write_pn(os, p[i]);
        }
    }

    template <typename... T>
    void write_pn(std::ostream& os,
                  const VertexArrayData<std::tuple<Xyz::Vector3F, Xyz::Vector3F, T...>>& data)
    {
        os << "Vertices:\n";
        write_pn(os, std::span(data.vertices), true);
        os << "Indices:\n";
        write_triangles(os, std::span(data.indices), true);
    }

    template <typename... T>
    void write_pnt(std::ostream& os,
                   const std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>& p)
    {
        os << std::get<0>(p) << " " << std::get<1>(p) << " " << std::get<2>(p) << '\n';
    }

    template <typename... T>
    void write_pnt(std::ostream& os,
                   std::span<const std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>> p,
                   bool indexes = true)
    {
        for (size_t i = 0; i < p.size(); ++i)
        {
            if (indexes)
                os << i << ": ";
            write_pnt(os, p[i]);
        }
    }

    template <typename... T>
    void write_pnt(
        std::ostream& os,
        const VertexArrayData<std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>>& data)
    {
        os << "Vertices:\n";
        write_pnt<T...>(os, std::span(data.vertices), true);
        os << "Indices:\n";
        write_triangles(os, std::span(data.indices), true);
    }

    /**
     * Adds a rectangle defined by position and normal vectors to the vertex
     * array data builder.
     *
     * The rectangle is added as two triangles.
     *
     * @tparam T Additional vertex attributes.
     * @param builder The vertex array data builder to add the rectangle to.
     * @param rect The rectangle to add.
     */
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

    /**
     * Adds a cube defined by position and normal vectors to the vertex
     * array data builder.
     *
     * The cube is added as 12 triangles (2 per face).
     *
     * @tparam T Additional vertex attributes.
     * @param builder The vertex array data builder to add the cube to.
     */
    template <typename... T>
    void add_cube_pn(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, T...>>& builder)
    {
        constexpr auto PI = Xyz::Constants<float>::PI;
        using O3F = Xyz::Orientation3F;
        auto pn = [](float x, float y, float z, float yaw, float roll)
        {
            return Xyz::Rectangle3F{
                {{x, y, z}, O3F{yaw, 0, roll}},
                {2, 2}
            };
        };
        add_rect_pn(builder, pn(-1, -1, 1, 0, 0));
        add_rect_pn(builder, pn(-1, 1, -1, -PI / 2, PI / 2));
        add_rect_pn(builder, pn(-1, -1, -1, 0, PI / 2));
        add_rect_pn(builder, pn(1, -1, -1, PI / 2, PI / 2));
        add_rect_pn(builder, pn(1, 1, -1, PI, PI / 2));
        add_rect_pn(builder, pn(-1, 1, -1, 0, PI));
    }

    /**
     * Adds a rectangle defined by position, normal and texture coordinate
     * vectors to the vertex array data builder.
     *
     * The rectangle is added as two triangles.
     *
     * @tparam T Additional vertex attributes.
     * @param builder The vertex array data builder to add the rectangle to.
     * @param rect The rectangle to add.
     * @param uv_rect The texture coordinates for the rectangle.
     */
    template <typename... T>
    void add_rect_pnt(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>>&
        builder,
        const Xyz::Rectangle3F& rect,
        const Xyz::Rectangle2F& uv_rect
    )
    {
        const auto base_index = builder.max_index();
        add_rect_pn(builder, rect);
        std::get<2>(builder.vertex(base_index + 0)) = uv_rect[0];
        std::get<2>(builder.vertex(base_index + 1)) = uv_rect[1];
        std::get<2>(builder.vertex(base_index + 2)) = uv_rect[2];
        std::get<2>(builder.vertex(base_index + 3)) = uv_rect[3];
    }

    /**
     * Adds a cube defined by position, normal and texture coordinate
     * vectors to the vertex array data builder.
     *
     * The texture coordinates are mapped so that each face gets a
     * unique part of the texture. The layout is as follows:
     *
     * ```
     * |Top(z=1)|Left(x=-1)|Front(y=-1)|Right(x=1)|Back(y=1)|Bottom(z=-1)|
     * ```
     *
     * The cube is added as 12 triangles (2 per face).
     *
     * @tparam T Additional vertex attributes.
     * @param builder The vertex array data builder to add the cube to.
     */
    template <typename... T>
    void add_cube_pnt(
        VertexArrayDataBuilder<std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F, T...>>&
        builder
    )
    {
        constexpr auto PI = Xyz::Constants<float>::PI;
        using O3F = Xyz::Orientation3F;
        auto pn = [](float x, float y, float z, float yaw, float roll)
        {
            return Xyz::Rectangle3F{
                {{x, y, z}, O3F{yaw, 0, roll}},
                {2, 2}
            };
        };
        auto t = [](float f)
        {
            return Xyz::Rectangle2F{{{f / 6, 0.f}}, {1.f / 6, 1}};
        };
        add_rect_pnt(builder, pn(-1, -1, 1, 0, 0), t(0));
        add_rect_pnt(builder, pn(-1, 1, -1, -PI / 2, PI / 2), t(1));
        add_rect_pnt(builder, pn(-1, -1, -1, 0, PI / 2), t(2));
        add_rect_pnt(builder, pn(1, -1, -1, PI / 2, PI / 2), t(3));
        add_rect_pnt(builder, pn(1, 1, -1, PI, PI / 2), t(4));
        add_rect_pnt(builder, pn(-1, 1, -1, 0, PI), t(5));
    }
}
