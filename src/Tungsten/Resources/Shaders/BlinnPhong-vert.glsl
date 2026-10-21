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

// The size of SkinBlock's joint array; must match MAX_SKIN_JOINTS in
// Skin.hpp, which is the size of the buffer range the renderer binds to it.
#ifndef MAX_JOINTS
#define MAX_JOINTS 128
#endif

// Attribute locations follow the fixed AttributeSemantic convention (§13 of
// docs/scene_graph_design.md): the enum order *is* the location table, so
// TEX_COORD_0 is 4, not "the next free slot".
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 4) in vec2 a_tex_coord;

// SKINNED is a compile-time variant rather than a runtime flag because it
// changes the vertex format: a skinned mesh carries these two attributes,
// and the skin block has to be bound for every draw that declares it.
#ifdef SKINNED
// Floats, not integers: the indices go through the same attribute pointers as
// everything else (see VertexLayoutBuilder).
layout (location = 7) in vec4 a_joints;
layout (location = 8) in vec4 a_weights;

// binding 3 — the item's skin: each joint's world transform times its inverse
// bind matrix. The matrices are in world space, so the renderer leaves
// u_model and u_normal_matrix at identity for a skinned item.
layout (std140) uniform SkinBlock
{
    mat4 u_joints[MAX_JOINTS];
};
#endif

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

// binding 2 — per-draw: world transform and its normal matrix. The block is
// 112 bytes: a 64-byte mat4, then a mat3 that std140 lays out as three
// columns padded to vec4, i.e. 48 bytes. That is exactly what RenderItem
// stores, so its data() uploads verbatim — and the block must stay exactly
// that size, because WebGL2 rejects a draw whose uniform block is not fully
// backed by its buffer.
layout (std140) uniform PerDraw
{
    mat4 u_model;
    mat3 u_normal_matrix;
};

out VS_OUT
{
    vec3 frag_pos;   // world space
    vec3 normal;     // world space
    vec2 texcoord;
} vs_out;

void main()
{
    vec4 position = vec4(a_position, 1.0);
    vec3 normal = a_normal;
#ifdef SKINNED
    mat4 skin = a_weights.x * u_joints[int(a_joints.x)]
                + a_weights.y * u_joints[int(a_joints.y)]
                + a_weights.z * u_joints[int(a_joints.z)]
                + a_weights.w * u_joints[int(a_joints.w)];
    position = skin * position;
    // Exact as long as the joints scale uniformly, which skeletons nearly
    // always do; the inverse transpose per vertex would cost far more.
    normal = mat3(skin) * normal;
#endif

    vec4 world_pos = u_model * position;
    vs_out.frag_pos = world_pos.xyz;
    vs_out.normal = u_normal_matrix * normal;
    vs_out.texcoord = a_tex_coord;
    gl_Position = u_projection * u_view * world_pos;
}