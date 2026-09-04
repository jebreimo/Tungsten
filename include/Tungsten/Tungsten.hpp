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
#include "Gl/IOglWrapper.hpp"
#include "Neo/BuiltinShaders.hpp"
#include "Neo/CameraComponent.hpp"
#include "Neo/ColorMaterials.hpp"
#include "Neo/FontManager.hpp"
#include "Neo/LightComponent.hpp"
#include "Neo/Material.hpp"
#include "Neo/Mesh.hpp"
#include "Neo/NodeHandle.hpp"
#include "Neo/RenderableComponent.hpp"
#include "Neo/Renderer.hpp"
#include "Neo/ResourceManager.hpp"
#include "Neo/Scene.hpp"
#include "Neo/ShaderPreprocessor.hpp"
#include "Neo/ShaderProgramBuilder.hpp"
#include "Neo/SnapshotBuilder.hpp"
#include "Neo/TextComponent.hpp"
#include "Neo/TextStyle.hpp"
#include "Neo/TextSystem.hpp"
#include "Neo/Texture.hpp"
#include "Neo/VertexLayoutBuilder.hpp"
#include "Render/VertexArrayObjectBuilder.hpp"
#include "Sdl/SdlApplication.hpp"
#include "TungstenException.hpp"
#include "YimageGl.hpp"
