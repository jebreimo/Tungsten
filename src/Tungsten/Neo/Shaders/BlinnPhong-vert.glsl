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
// be known at compile time and must match the fragment shader. The number of
// lights actually used is controlled at runtime by u_light_count.
#ifndef MAX_LIGHTS
#define MAX_LIGHTS 8
#endif

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

// One unified light, mirroring RenderSnapshot's LightData. Packed for std140:
//   position.xyz  light position (point/spot)
//   position.w    type: 0 = directional, 1 = point, 2 = spot
//   direction.xyz light direction (directional/spot)
//   direction.w   range (point/spot)
//   color.rgb     light colour
//   color.w       intensity (colour is multiplied by this)
//   cone.x        cos(inner cone angle)   (spot)
//   cone.y        cos(outer cone angle)   (spot)
// The struct and the PerFrame block must be declared identically in both
// stages (GLSL has no include); the vertex stage only reads the matrices.
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

// binding 2 — per-draw: world transform and its normal matrix. Only the
// upper-left 3x3 of u_normal_matrix is used; it is a mat4 so the C++ side can
// upload it without std140 per-column padding.
layout (std140) uniform PerDraw
{
    mat4 u_model;
    mat4 u_normal_matrix;
};

out VS_OUT
{
    vec3 frag_pos;   // world space
    vec3 normal;     // world space
    vec2 texcoord;
} vs_out;

void main()
{
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    vs_out.frag_pos = world_pos.xyz;
    vs_out.normal = mat3(u_normal_matrix) * a_normal;
    vs_out.texcoord = a_tex_coord;
    gl_Position = u_projection * u_view * world_pos;
}