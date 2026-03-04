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
    std::vector<ShaderFeature> SmoothShader::get_features()
    {
        return {
            {"diffuse map", "Use diffuse map (texture)", "USE_DIFFUSE_MAP", {}},
            {"specular map", "Use specular map", "USE_SPECULAR_MAP", {}},
            {"directional_light", "Use directional light", "USE_DIRECTIONAL_LIGHT", {}},
            {"point_lights", "Use point lights", "USE_POINT_LIGHTS", {}},
            {"spotlight", "Use spotlight", "USE_SPOTLIGHT", {}}
        };
    }

    SmoothShader::SmoothShader()
        : ShaderProgram(std::string(NAME),
                        {
                            {ShaderType::VERTEX, ShaderSources::BLINN_PHONG_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::BLINN_PHONG_FRAGMENT}
                        },
                        ShaderPreprocessor().add_define("USE_DIRECTIONAL_LIGHT")),
          material("u_material.", *this),
          directional_light("u_dir_light.", *this)
    {
        std::vector<VertexAttributeDefinition> attr_defs;
        position_attr = get_vertex_attribute(id(), "a_position");
        attr_defs.emplace_back(position_attr, VertexAttributeType::POSITION_3F);
        normal_attr = get_vertex_attribute(id(), "a_normal");
        attr_defs.emplace_back(normal_attr, VertexAttributeType::NORMAL_3F);
        texture_coord_attr = get_vertex_attribute(id(), "a_tex_coord");
        if (texture_coord_attr != INVALID_VERTEX_ATTRIBUTE)
            attr_defs.emplace_back(texture_coord_attr, VertexAttributeType::TEX_COORD_2F);
        set_attribute_definitions(std::move(attr_defs));

        model_view_matrix = get_uniform<Xyz::Matrix4F>(id(), "u_model_view_matrix");
        normal_matrix = get_uniform<Xyz::Matrix3F>(id(), "u_normal_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(id(), "u_proj_matrix");
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
