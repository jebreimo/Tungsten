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
    struct Mesh;
    using MeshRef = ResourceRef<Mesh>;
    struct Material;
    using MaterialRef = ResourceRef<Material>;
    struct ShaderProgram;
    using ShaderProgramRef = ResourceRef<ShaderProgram>;
    struct Texture;
    using TextureRef = ResourceRef<Texture>;
    class BufferArena;
    using BufferArenaRef = ResourceRef<BufferArena>;
    struct VertexLayout;
    using VertexLayoutRef = ResourceRef<VertexLayout>;
    struct SamplerDescriptor;
    using SamplerRef = ResourceRef<SamplerDescriptor>;
}
