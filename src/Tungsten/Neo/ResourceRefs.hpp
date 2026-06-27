//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-25.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ResourceRef.hpp"

namespace Tungsten
{
    class Mesh;
    using MeshRef = ResourceRef<Mesh>;
    class Material;
    using MaterialRef = ResourceRef<Material>;
    class ShaderProgram;
    using ShaderProgramRef = ResourceRef<ShaderProgram>;
    class Texture;
    using TextureRef = ResourceRef<Texture>;
    class BufferArena;
    using BufferArenaRef = ResourceRef<BufferArena>;
    class VertexLayout;
    using VertexLayoutRef = ResourceRef<VertexLayout>;
}
