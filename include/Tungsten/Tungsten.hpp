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
#include "Gl/GlStateEpoch.hpp"
#include "Gl/GlStateManagement.hpp"
#include "Gl/GlTexture.hpp"
#include "Gl/GlTypes.hpp"
#include "Gl/GlUniform.hpp"
#include "Gl/GlVertexArray.hpp"
#include "Gl/IOglWrapper.hpp"
#include "Rendering/DoubleBuffer.hpp"
#include "Rendering/FontManager.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/SnapshotBuilder.hpp"
#include "Rendering/TextStyle.hpp"
#include "Rendering/TextSystem.hpp"
#include "Resources/BuiltinShaders.hpp"
#include "Resources/ColorMaterials.hpp"
#include "Resources/Material.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/ShaderPreprocessor.hpp"
#include "Resources/ShaderProgramBuilder.hpp"
#include "Resources/Texture.hpp"
#include "Resources/VertexArray.hpp"
#include "Resources/VertexLayoutBuilder.hpp"
#include "SceneGraph/CameraComponent.hpp"
#include "SceneGraph/LightComponent.hpp"
#include "SceneGraph/NodeHandle.hpp"
#include "SceneGraph/RenderableComponent.hpp"
#include "SceneGraph/Scene.hpp"
#include "SceneGraph/TextComponent.hpp"
#include "Sdl/SdlApplication.hpp"
#include "TungstenException.hpp"
#include "YimageGl.hpp"
