//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

// MAX_LIGHTS sizes the unified light array in the per-frame block, so it must
// be known at compile time and must match the vertex shader. The number of
// lights actually used is controlled at runtime by u_light_count.
#ifndef MAX_LIGHTS
#define MAX_LIGHTS 8
#endif

const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

layout (location = 0) out vec4 color;

in VS_OUT
{
    vec3 frag_pos;   // world space
    vec3 normal;     // world space
    vec2 texcoord;
} fs_in;

// One unified light, mirroring RenderSnapshot's LightData. Packed for std140:
//   position.xyz  light position (point/spot)
//   position.w    type: 0 = directional, 1 = point, 2 = spot
//   direction.xyz light direction (directional/spot)
//   direction.w   range (point/spot)
//   color.rgb     light colour
//   color.w       intensity (colour is multiplied by this)
//   cone.x        cos(inner cone angle)   (spot)
//   cone.y        cos(outer cone angle)   (spot)
struct Light
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 cone;
};

// binding 0 — per-frame: camera, ambient, lights, time. Bound once per frame.
layout (std140) uniform PerFrame
{
    mat4 u_view;
    mat4 u_projection;
    vec4 u_camera_pos;       // xyz = world-space camera position, w = time
    vec4 u_ambient_light;    // rgb = global ambient colour
    Light u_lights[MAX_LIGHTS];
    ivec4 u_light_count;      // x = number of active lights
};

// binding 1 — per-material: the Blinn-Phong parameter blob. Bound on material
// change. The map flags select between the constant colours below and the
// sampled maps; the samplers themselves cannot live in a UBO.
layout (std140) uniform MaterialBlock
{
    vec4 u_ambient;          // rgb
    vec4 u_diffuse;          // rgb
    vec4 u_specular;         // rgb, w = shininess
    ivec4 u_material_flags;  // x = use_diffuse_map, y = use_specular_map
};

uniform sampler2D u_diffuse_map;
uniform sampler2D u_specular_map;

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

    if (u_material_flags.x != 0)
    {
        // Ambient tracks the albedo; the global ambient light scales it.
        vec3 diffuse = vec3(texture(u_diffuse_map, fs_in.texcoord));
        material.ambient = diffuse;
        material.diffuse = diffuse;
    }
    else
    {
        material.ambient = u_ambient.rgb;
        material.diffuse = u_diffuse.rgb;
    }

    material.specular = u_material_flags.y != 0
        ? vec3(texture(u_specular_map, fs_in.texcoord))
        : u_specular.rgb;

    material.shininess = u_specular.w;
    return material;
}

// Smooth, range-bounded inverse-square falloff (Frostbite / UE style): a
// physical 1/d^2 windowed so it reaches exactly 0 at the light's range.
float distance_attenuation(float dist2, float range)
{
    float att = 1.0 / max(dist2, 1e-4);
    float window = clamp(1.0 - pow(dist2 / (range * range), 2.0), 0.0, 1.0);
    return att * window * window;
}

vec3 calc_light(Light light, ColorMaterial material,
                vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    int type = int(light.position.w);

    vec3 light_dir;
    float attenuation = 1.0;

    if (type == LIGHT_DIRECTIONAL)
    {
        light_dir = normalize(-light.direction.xyz);
    }
    else
    {
        vec3 to_light = light.position.xyz - frag_pos;
        light_dir = normalize(to_light);
        attenuation = distance_attenuation(dot(to_light, to_light),
                                           light.direction.w);

        if (type == LIGHT_SPOT)
        {
            // Soft edge between the inner and outer cone.
            float theta = dot(light_dir, normalize(-light.direction.xyz));
            float epsilon = max(light.cone.x - light.cone.y, 1e-4);
            attenuation *= clamp((theta - light.cone.y) / epsilon, 0.0, 1.0);
        }
    }

    // A light emits one colour; intensity scales it. The Blinn-Phong split
    // lives in the material's diffuse/specular reflectance, not in the light.
    vec3 radiance = light.color.rgb * light.color.w;

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 halfway = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway), 0.0), material.shininess);

    vec3 diffuse = radiance * diff * material.diffuse;
    vec3 specular = radiance * spec * material.specular;
    return (diffuse + specular) * attenuation;
}

void main()
{
    vec3 normal = normalize(fs_in.normal);
    vec3 view_dir = normalize(u_camera_pos.xyz - fs_in.frag_pos);

    ColorMaterial material = get_material();

    // Global ambient is applied once, not once per light.
    vec3 result = material.ambient * u_ambient_light.rgb;

    for (int i = 0; i < u_light_count.x; i++)
        result += calc_light(u_lights[i], material, normal, fs_in.frag_pos, view_dir);

    color = vec4(result, 1.0);
}