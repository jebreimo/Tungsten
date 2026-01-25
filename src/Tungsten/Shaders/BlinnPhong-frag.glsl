//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

layout (location = 0) out vec4 color;

in VS_OUT
{
    vec3 frag_pos;
    vec3 normal;
    #ifdef USE_TEXTURES
    vec2 texcoord;
    #endif
} fs_in;

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 u_view_pos;
uniform Material u_material;
uniform DirectionalLight u_dir_light;
//uniform sampler2D u_texture;

void main()
{
    vec3 ambient = u_dir_light.ambient * u_material.ambient;
    // texture(material.diffuse, TexCoords).rgb;

    // Directional light
    vec3 norm = normalize(fs_in.normal);
    vec3 light_dir = normalize(-u_dir_light.direction);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = u_dir_light.diffuse * diff * u_material.diffuse;

    vec3 view_dir = normalize(u_view_pos - fs_in.frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);
    vec3 specular = u_dir_light.specular * spec * u_material.specular;
    //texture(material.specular, TexCoords).rgb;

    color = vec4(ambient + diffuse + specular, 1.0);
}
