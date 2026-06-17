//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/SmoothShader.hpp"

#include "Tungsten/VertexArrayObjectBuilder.hpp"
#include "../ShaderSources.hpp"

namespace Tungsten
{
    SmoothShader::SmoothShader()
        : ShaderProgram(std::string(NAME),
                        {
                            {ShaderType::VERTEX, ShaderSources::BLINN_PHONG_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::BLINN_PHONG_FRAGMENT}
                        }),
          material("u_material.", *this),
          directional_light("u_dir_light.", *this)
    {
        std::vector<VertexAttributeDefinition> attr_defs;
        position_attr = get_vertex_attribute(id(), "a_position");
        attr_defs.emplace_back(position_attr, VertexAttributeType::POSITION_3F);
        normal_attr = get_vertex_attribute(id(), "a_normal");
        attr_defs.emplace_back(normal_attr, VertexAttributeType::NORMAL_3F);
        // a_tex_coord is always present in the shader now, but is only fed
        // real data when a mesh carries texture coordinates. It is left out
        // of the default vertex format (position + normal); callers that use
        // material maps add it to their vertex layout explicitly.
        texture_coord_attr = get_vertex_attribute(id(), "a_tex_coord");
        set_attribute_definitions(std::move(attr_defs));

        model_view_matrix = get_uniform<Xyz::Matrix4F>(id(), "u_model_view_matrix");
        normal_matrix = get_uniform<Xyz::Matrix3F>(id(), "u_normal_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(id(), "u_proj_matrix");

        use_diffuse_map = get_uniform<int32_t>(id(), "u_use_diffuse_map");
        use_specular_map = get_uniform<int32_t>(id(), "u_use_specular_map");
        use_directional_light = get_uniform<int32_t>(id(), "u_use_directional_light");
        num_point_lights = get_uniform<int32_t>(id(), "u_num_point_lights");
        use_spot_light = get_uniform<int32_t>(id(), "u_use_spot_light");

        // Default to the previous compile-time configuration: a single
        // directional light lighting a colour-only material.
        use();
        set_use_diffuse_map(false);
        set_use_specular_map(false);
        set_use_directional_light(true);
        set_num_point_lights(0);
        set_use_spot_light(false);
    }

    void SmoothShader::set_use_diffuse_map(bool enabled)
    {
        use_diffuse_map.set(enabled ? 1 : 0);
    }

    void SmoothShader::set_use_specular_map(bool enabled)
    {
        use_specular_map.set(enabled ? 1 : 0);
    }

    void SmoothShader::set_use_directional_light(bool enabled)
    {
        use_directional_light.set(enabled ? 1 : 0);
    }

    void SmoothShader::set_num_point_lights(int count)
    {
        num_point_lights.set(count);
    }

    void SmoothShader::set_use_spot_light(bool enabled)
    {
        use_spot_light.set(enabled ? 1 : 0);
    }

    void SmoothShader::set_model_view_matrix(const Xyz::Matrix4F& mv,
                                             bool update_normal_matrix)
    {
        model_view_matrix.set(mv);
        if (update_normal_matrix)
        {
            normal_matrix.set(Xyz::make_submatrix<3, 3>(invert(mv), {}), false);
        }
    }

    void SmoothShader::set_model_view_matrix(const Xyz::Matrix4F& model,
                                             const Xyz::Matrix4F& view,
                                             bool update_normal_matrix)
    {
        set_model_view_matrix(view * model, update_normal_matrix);
    }

    void SmoothShader::set_projection_matrix(const Xyz::Matrix4F& proj)
    {
        proj_matrix.set(proj);
    }

    void SmoothShader::set_normal_matrix(const Xyz::Matrix3F& norm)
    {
        normal_matrix.set(norm);
    }
}
