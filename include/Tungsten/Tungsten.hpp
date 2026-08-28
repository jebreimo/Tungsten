//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include "DeviceInfo.hpp"
#include "Gl/GlBuffer.hpp"
#include "Gl/GlFramebuffer.hpp"
#include "Gl/GlProgram.hpp"
#include "Gl/GlRendering.hpp"
#include "Gl/GlStateManagement.hpp"
#include "Gl/GlTexture.hpp"
#include "Gl/GlTypes.hpp"
#include "Gl/GlUniform.hpp"
#include "Gl/IOglWrapper.hpp"
#include "Render/Camera.hpp"
#include "Render/ColorMaterials.hpp"
#include "Render/FontManager.hpp"
#include "Render/ShaderManager.hpp"
#include "Render/ShaderProgramBuilder.hpp"
#include "Render/ShaderTools.hpp"
#include "Render/TextRenderer.hpp"
#include "Render/VertexArrayObjectBuilder.hpp"
#include "Sdl/SdlApplication.hpp"
#include "Tungsten/Neo/BuiltinShaders.hpp"
#include "Tungsten/Neo/CameraComponent.hpp"
#include "Tungsten/Neo/LightComponent.hpp"
#include "Tungsten/Neo/Material.hpp"
#include "Tungsten/Neo/Mesh.hpp"
#include "Tungsten/Neo/NodeHandle.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/Renderer.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/Scene.hpp"
#include "Tungsten/Neo/SnapshotBuilder.hpp"
#include "Tungsten/Neo/TextComponent.hpp"
#include "Tungsten/Neo/TextStyle.hpp"
#include "Tungsten/Neo/TextSystem.hpp"
#include "Tungsten/Neo/Texture.hpp"
#include "TungstenException.hpp"
#include "YimageGl.hpp"
