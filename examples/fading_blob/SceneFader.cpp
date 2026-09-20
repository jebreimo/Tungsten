//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SceneFader.hpp"

#include <array>
#include <cstring>
#include <Tungsten/Tungsten.hpp>
#include "Resources.hpp"

using namespace Tungsten;

namespace
{
    // The fader's own family, one past the blob's.
    constexpr ShaderFamilyId FADER_FAMILY = FIRST_USER_SHADER_FAMILY + 1;

    constexpr uint32_t VERTEX_COUNT = 4;
    constexpr uint32_t INDEX_COUNT = 6;
    // 2 position floats + 2 texture-coordinate floats.
    constexpr uint16_t VERTEX_STRIDE = 4 * sizeof(float);

    std::vector<std::byte> make_color_delta(float step)
    {
        const float values[4] = {step, step, step, 0.0f};
        std::vector<std::byte> blob(sizeof(values));
        std::memcpy(blob.data(), values, sizeof(values));
        return blob;
    }
}

class SceneFader::SceneFaderImpl
{
public:
    SceneFaderImpl(ResourceManager& resources, Size2I window_size)
        : resources_(resources),
          builder_(resources)
    {
        register_family();

        PipelineDescriptor descriptor;
        descriptor.shader = resources_.register_shader_variant({FADER_FAMILY, 0});
        descriptor.layout = quad_layout();
        // A full-screen quad has no meaningful depth and must never be
        // rejected by whatever the scene left in the depth buffer. Culling is
        // off for the same reason a glyph quad's is: its winding is nobody's
        // contract.
        descriptor.depth = {.test = false, .write = false};
        descriptor.raster.cull_enabled = false;
        pipeline_ = resources_.register_pipeline(descriptor);

        build_quad();

        // One node holding the quad, plus the camera SnapshotBuilder needs.
        // The fader's shader declares neither PerFrame nor PerDraw — it emits
        // clip coordinates directly — so the camera's values never matter.
        quad_node_ = scene_.add_node();
        quad_node_.add(RenderableComponent{.mesh = quad_});
        camera_node_ = scene_.add_node();
        camera_node_.add(CameraComponent{});

        set_window_size(window_size);
    }

    ~SceneFaderImpl()
    {
        release_buffers();
    }

    void set_window_size(Size2I size)
    {
        if (size == size_ && targets_[0])
            return;

        release_buffers();
        size_ = size;

        for (size_t i = 0; i < 2; ++i)
        {
            textures_[i] = resources_.create_texture(TextureImage2D{
                .size = size,
                .format = RGB_TEXTURE,
                .content = TextureContent::COLOR,
                .pixels = nullptr,
                .sampler = sampler_
            });
            targets_[i] = resources_.create_render_target(textures_[i]);
        }

        // Each material samples the *other* buffer, which is the one drawn
        // last frame. Textures are fixed at material creation, so the pair is
        // built once here rather than re-pointed per frame.
        for (size_t i = 0; i < 2; ++i)
        {
            fade_materials_[i] = make_material(textures_[1 - i], fade_step_);
            present_materials_[i] = make_material(textures_[i], 0.0f);
        }

        // Fresh texture storage holds undefined data and both buffers are
        // sampled before either is drawn to, so clear them: the first frame
        // should fade in from black, not from whatever was in memory.
        cleared_ = false;
    }

    void draw_previous_scene(Renderer& renderer, float fade_step)
    {
        if (fade_step != fade_step_)
        {
            fade_step_ = fade_step;
            for (size_t i = 0; i < 2; ++i)
            {
                resources_.update_material_parameters(
                    fade_materials_[i], make_color_delta(-fade_step_));
            }
        }

        if (!cleared_)
        {
            // Both buffers start black, so the trail fades in from nothing.
            for (const auto target : targets_)
                clear_target(renderer, target);
            cleared_ = true;
        }

        // The quad covers every pixel, so the old contents need not be read
        // back: DONT_CARE is free here and saves a tile load on a tiler.
        draw_quad(renderer, fade_materials_[index_],
                  pass_for(targets_[index_], LoadAction::DONT_CARE));
    }

    [[nodiscard]]
    RenderPassDescriptor accumulation_pass(const Viewport& viewport) const
    {
        auto pass = pass_for(targets_[index_], LoadAction::LOAD);
        pass.viewport = viewport;
        return pass;
    }

    void render_scene(Renderer& renderer, const Viewport& viewport)
    {
        RenderPassDescriptor pass;
        pass.viewport = viewport;
        pass.color.load = LoadAction::DONT_CARE;
        pass.depth.reset();
        draw_quad(renderer, present_materials_[index_], pass);

        index_ = 1 - index_;
    }

private:
    void register_family()
    {
        ShaderFamily family;
        family.vertex_source = SCENE_FADER_VERTEX;
        family.fragment_source = SCENE_FADER_FRAGMENT;
        family.samplers = {{"u_texture", TextureContent::COLOR}};
        family.required_attributes
            = semantic_bit(AttributeSemantic::POSITION)
              | semantic_bit(AttributeSemantic::TEX_COORD_0);
        resources_.register_shader_family(FADER_FAMILY, std::move(family));
    }

    VertexLayoutRef quad_layout()
    {
        // POSITION defaults to three components; the quad is flat, so it
        // carries two, with the texture coordinate packed after it.
        return resources_.register_layout(
            VertexLayoutBuilder()
                .add_attribute(AttributeSemantic::POSITION)
                .set_component_count(2)
                .add_attribute(AttributeSemantic::TEX_COORD_0)
                .build());
    }

    void build_quad()
    {
        constexpr float vertexes[4 * VERTEX_COUNT] = {
            -1, -1, 0, 0,
            1, -1, 1, 0,
            1, 1, 1, 1,
            -1, 1, 0, 1
        };
        uint16_t indexes[INDEX_COUNT] = {0, 1, 2, 0, 2, 3};

        vbo_arena_ = resources_.create_arena(BufferUsage::STATIC_DRAW,
                                             VERTEX_STRIDE, VERTEX_COUNT);
        ebo_arena_ = resources_.create_arena(BufferUsage::STATIC_DRAW,
                                             sizeof(uint16_t), INDEX_COUNT);

        const auto vertices = resources_.allocate(vbo_arena_, VERTEX_COUNT);
        const auto indices = resources_.allocate(ebo_arena_, INDEX_COUNT);

        // Rebase to absolute indices (§3): the portable path has no
        // baseVertex draw argument.
        for (auto& index : indexes)
            index = uint16_t(index + vertices.offset);

        resources_.upload(vertices, vertexes, sizeof(vertexes));
        resources_.upload(indices, indexes, sizeof(indexes));

        Mesh mesh;
        mesh.streams = {vertices};
        mesh.layout = quad_layout();
        mesh.ebo = indices;
        mesh.index_type = ElementIndexType::UINT16;
        quad_ = resources_.create_mesh(std::move(mesh));
    }

    MaterialRef make_material(TextureRef texture, float step)
    {
        Material material;
        material.pipeline = pipeline_;
        material.parameter_data = make_color_delta(-step);
        material.textures = {texture};
        return resources_.create_material(std::move(material));
    }

    [[nodiscard]]
    RenderPassDescriptor pass_for(RenderTargetRef target,
                                  LoadAction load) const
    {
        RenderPassDescriptor pass;
        pass.target = target;
        pass.viewport = {{0, 0}, Xyz::vector_cast<float>(size_)};
        pass.color.load = load;
        // The accumulation buffers carry no depth attachment, so there is
        // nothing for a depth action to apply to.
        pass.depth.reset();
        return pass;
    }

    void clear_target(Renderer& renderer, RenderTargetRef target)
    {
        auto pass = pass_for(target, LoadAction::CLEAR);
        pass.color.clear_color = {0, 0, 0, 1};
        // An empty snapshot: the pass's clear is the whole point.
        scene_.resolve_transforms();
        quad_node_.get<RenderableComponent>().visible = false;
        builder_.build(scene_, camera_node_.id(), snapshot_);
        renderer.render(snapshot_, pass);
        quad_node_.get<RenderableComponent>().visible = true;
    }

    void draw_quad(Renderer& renderer, MaterialRef material,
                   const RenderPassDescriptor& pass)
    {
        quad_node_.get<RenderableComponent>().material = material;
        scene_.resolve_transforms();
        builder_.build(scene_, camera_node_.id(), snapshot_);
        renderer.render(snapshot_, pass);
    }

    void release_buffers()
    {
        for (auto& material : fade_materials_)
        {
            if (material)
                resources_.destroy_material(std::exchange(material, {}));
        }
        for (auto& material : present_materials_)
        {
            if (material)
                resources_.destroy_material(std::exchange(material, {}));
        }
        for (auto& target : targets_)
        {
            if (target)
                resources_.destroy_render_target(std::exchange(target, {}));
        }
        for (auto& texture : textures_)
        {
            if (texture)
                resources_.destroy_texture(std::exchange(texture, {}));
        }
    }

    ResourceManager& resources_;
    Scene scene_;
    SnapshotBuilder builder_;
    RenderSnapshot snapshot_;
    NodeHandle quad_node_;
    NodeHandle camera_node_;

    BufferArenaRef vbo_arena_;
    BufferArenaRef ebo_arena_;
    MeshRef quad_;
    PipelineRef pipeline_;
    SamplerRef sampler_;

    std::array<TextureRef, 2> textures_;
    std::array<RenderTargetRef, 2> targets_;
    std::array<MaterialRef, 2> fade_materials_;
    std::array<MaterialRef, 2> present_materials_;

    Size2I size_;
    float fade_step_ = 0.0f;
    bool cleared_ = false;
    int index_ = 0;
};

SceneFader::SceneFader(ResourceManager& resources, Size2I window_size)
    : impl_(std::make_unique<SceneFaderImpl>(resources, window_size))
{
}

SceneFader::~SceneFader() = default;

void SceneFader::set_window_size(Size2I window_size)
{
    impl_->set_window_size(window_size);
}

void SceneFader::draw_previous_scene(Renderer& renderer, float fade_step)
{
    impl_->draw_previous_scene(renderer, fade_step);
}

RenderPassDescriptor SceneFader::accumulation_pass(
    const Viewport& viewport) const
{
    return impl_->accumulation_pass(viewport);
}

void SceneFader::render_scene(Renderer& renderer, const Viewport& viewport)
{
    impl_->render_scene(renderer, viewport);
}
