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
    vec3 view_pos;
    vec3 normal;
    #ifdef USE_TEXTURES
    vec2 texcoord;
    #endif
} fs_in;

struct ColorMaterial
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform ColorMaterial u_col_material;

#ifdef USE_DIRECTIONAL_LIGHT

struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight u_dir_light;

vec3 calc_dir_light(DirectionalLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), u_col_material.shininess);
    // combine results
    vec3 ambient = light.ambient * u_col_material.ambient;
    vec3 diffuse = light.diffuse * diff * u_col_material.diffuse;
    vec3 specular = light.specular * spec * u_col_material.specular;
    //    vec3 ambient = light.ambient * vec3(texture(u_col_material.diffuse, fs_in.texcoord));
    //    vec3 diffuse = light.diffuse * diff * vec3(texture(u_col_material.diffuse, fs_in.texcoord));
    //    vec3 specular = light.specular * spec * vec3(texture(u_col_material.specular, fs_in.texcoord));
    return (ambient + diffuse + specular);
}

#endif

#ifdef USE_POINT_LIGHTS

#ifndef MAX_POINT_LIGHTS
#define MAX_POINT_LIGHTS 4
#endif

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

// calculates the color when using a point light.
vec3 calc_point_light(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), u_col_material.shininess);
    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * u_col_material.diffuse;
    vec3 diffuse = light.diffuse * diff * u_col_material.diffuse;
    vec3 specular = light.specular * spec * u_col_material.specular;
    //    vec3 ambient = light.ambient * vec3(texture(u_col_material.diffuse, fs_in.texcoord));
    //    vec3 diffuse = light.diffuse * diff * vec3(texture(u_col_material.diffuse, fs_in.texcoord));
    //    vec3 specular = light.specular * spec * vec3(texture(u_col_material.specular, fs_in.texcoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_num_point_lights;

#endif

#ifdef USE_SPOT_LIGHT

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

// calculates the color when using a spot light.
vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), u_col_material.shininess);
    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * u_col_material.diffuse;
    vec3 diffuse = light.diffuse * diff * u_col_material.diffuse;
    vec3 specular = light.specular * spec * u_col_material.specular;
    //    vec3 ambient = light.ambient * vec3(texture(u_col_material.diffuse, fs_in.texcoord));
    //    vec3 diffuse = light.diffuse * diff * vec3(texture(u_col_material.diffuse, fs_in.texcoord));
    //    vec3 specular = light.specular * spec * vec3(texture(u_col_material.specular, fs_in.texcoord));

    // spotlight intensity
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

uniform SpotLight u_spot_light;

#endif

void main()
{
    vec3 normal = normalize(fs_in.normal);
    vec3 view_dir = normalize(fs_in.view_pos);

    vec3 result = vec3(0.0);

    #ifdef USE_DIRECTIONAL_LIGHT
    result += calc_dir_light(u_dir_light, normal, view_dir);
    #endif

    #ifdef USE_POINT_LIGHTS
    for (int i = 0; i < u_num_point_lights; i++)
        result += calc_point_light(u_point_lights[i], normal, fs_in.frag_pos, view_dir);
    #endif

    #ifdef USE_SPOT_LIGHT
    result += calc_spot_light(u_spot_light, normal, fs_in.frag_pos, view_dir);
    #endif

    color = vec4(result, 1.0);
}
