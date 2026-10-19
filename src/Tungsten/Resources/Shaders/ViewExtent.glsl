//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-19.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// The distance from the camera to the furthest corners of the view frustum,
// i.e. the corners of the far plane, read from the projection matrix. A
// geometry that reaches this far from the camera in every direction covers
// everything the camera can see.
//
// Handles the symmetric perspective and orthographic matrices that
// CameraComponent produces. A perspective matrix with an infinite far plane
// gives an infinite distance.
float view_extent(mat4 projection)
{
    // The half width and height of the view: at unit distance for a
    // perspective projection, and everywhere for an orthographic one.
    vec2 half_size = 1.0 / vec2(projection[0][0], projection[1][1]);

    // The bottom row is (0, 0, -1, 0) in a perspective projection and
    // (0, 0, 0, 1) in an orthographic one.
    if (projection[2][3] != 0.0)
    {
        float far_plane = projection[3][2] / (projection[2][2] + 1.0);
        return far_plane * length(vec3(half_size, 1.0));
    }

    float far_plane = (projection[3][2] - 1.0) / projection[2][2];
    return length(vec3(half_size, far_plane));
}
