//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-18.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

// MAX_POINT_LIGHTS sizes the point-light array, so it must be known at
// compile time. Everything else is controlled at runtime by the uniform
// flags below.
#ifndef MAX_POINT_LIGHTS
#define MAX_POINT_LIGHTS 4
#endif

layout (location = 0) out vec4 color;

in VS_OUT
{
    vec3 frag_pos;
    vec3 view_pos;
    vec3 normal;
    vec2 texcoord;
} fs_in;

struct Material
{
    sampler2D diffuse_map;
    sampler2D specular_map;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material u_material;
uniform bool u_use_diffuse_map;
uniform bool u_use_specular_map;

struct ColorMaterial
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

ColorMaterial get_material()
{
    ColorMaterial material;

    if (u_use_diffuse_map)
    {
        vec3 diffuse = vec3(texture(u_material.diffuse_map, fs_in.texcoord));
        material.ambient = diffuse * 0.1;
        material.diffuse = diffuse;
    }
    else
    {
        material.ambient = u_material.ambient;
        material.diffuse = u_material.diffuse;
    }

    material.specular = u_use_specular_map
        ? vec3(texture(u_material.specular_map, fs_in.texcoord))
        : u_material.specular;

    material.shininess = u_material.shininess;
    return material;
}

struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight u_dir_light;
uniform bool u_use_directional_light;

vec3 calc_dir_light(DirectionalLight light, ColorMaterial material,
                    vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), material.shininess);

    // combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

struct PointLight
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_num_point_lights;

// calculates the color when using a point light.
vec3 calc_point_light(PointLight light, ColorMaterial material,
                      vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cut_off;
    float outer_cut_off;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform SpotLight u_spot_light;
uniform bool u_use_spot_light;

// calculates the color when using a spot light.
vec3 calc_spot_light(SpotLight light, ColorMaterial material,
                     vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = normalize(fs_in.normal);
    vec3 view_dir = normalize(fs_in.view_pos);

    ColorMaterial material = get_material();
    vec3 result = vec3(0.0);

    if (u_use_directional_light)
        result += calc_dir_light(u_dir_light, material, normal, view_dir);

    for (int i = 0; i < u_num_point_lights; i++)
        result += calc_point_light(u_point_lights[i], material, normal, fs_in.frag_pos, view_dir);

    if (u_use_spot_light)
        result += calc_spot_light(u_spot_light, material, normal, fs_in.frag_pos, view_dir);

    color = vec4(result, 1.0);
}