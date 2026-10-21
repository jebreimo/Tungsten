//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
#include <vector>
#include <Xyz/Matrix.hpp>
#include "NodeId.hpp"

namespace Tungsten
{
    /**
     * The most joints one skin can have. Every skinned draw binds a uniform
     * block of exactly this many matrices, so it is also the size of that
     * block: WebGL2 rejects a draw whose block is not fully backed. It must
     * match MAX_JOINTS in BlinnPhong-vert.glsl.
     */
    inline constexpr size_t MAX_SKIN_JOINTS = 128;

    /**
     * The skeleton that deforms a skinned mesh: the nodes whose world
     * transforms move its vertices, and for each one the matrix that takes
     * a vertex from the mesh's space into the joint's space in the bind pose.
     *
     * A vertex's JOINTS attribute indexes joints, so the order matters. A
     * skin is shared by every renderable it deforms, which is what lets the
     * snapshot compute its joint matrices once per frame rather than once
     * per renderable.
     */
    struct Skin
    {
        std::vector<NodeId> joints;
        std::vector<Xyz::Matrix4F> inverse_bind_matrices;
    };
} // Tungsten
