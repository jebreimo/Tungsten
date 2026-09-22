//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Import/GltfImport.hpp"

#include <bit>
#include <map>
#include <string>
#include <variant>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <Xyz/InvertMatrix.hpp>
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/YimageGl.hpp"
#include "Tungsten/Resources/BuiltinShaders.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/PipelineDescriptor.hpp"
#include "Tungsten/Resources/Texture.hpp"
#include "Tungsten/Resources/VertexLayoutBuilder.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"
#include "GltfConversion.hpp"

// Lets the accessor tools widen glTF's packed attribute types straight into
// Xyz vectors. Without these the importer would have to read fastgltf's own
// vector types and copy again, and would have to widen the normalized 8- and
// 16-bit attributes itself — Tungsten has no vertex format for those.
template <>
struct fastgltf::ElementTraits<Xyz::Vector3F>
    : fastgltf::ElementTraitsBase<Xyz::Vector3F, fastgltf::AccessorType::Vec3, float>
{};

template <>
struct fastgltf::ElementTraits<Xyz::Vector2F>
    : fastgltf::ElementTraitsBase<Xyz::Vector2F, fastgltf::AccessorType::Vec2, float>
{};

namespace Tungsten
{
    namespace
    {
        using Detail::GLTF_VERTEX_STRIDE;
        using Detail::PrimitiveData;

        /**
         * The capacity an arena needs to hold allocations of @a sizes without
         * ever growing.
         *
         * The arenas are sub-allocated by a buddy allocator, which rounds
         * every allocation up to a power of two, so the sum of the sizes is
         * not enough — the sum of the rounded sizes is. Sizing for the whole
         * file up front is what the pre-pass over the primitives buys: growing
         * an arena copies the entire buffer through glCopyBufferSubData, which
         * macOS's GL driver reports as unimplemented.
         */
        uint32_t required_capacity(const std::vector<uint32_t>& sizes)
        {
            uint32_t total = 0;
            for (const auto size : sizes)
                total += std::bit_ceil(std::max(size, 1u));
            return std::bit_ceil(std::max(total, 1u));
        }

        /**
         * The warnings as trailing lines of an error message. When an import
         * fails because everything in it was skipped, the reasons it was
         * skipped are the explanation the user actually needs.
         */
        std::string describe(const std::vector<std::string>& warnings)
        {
            std::string result;
            for (const auto& warning : warnings)
                result += "\n  - " + warning;
            return result;
        }

        /**
         * What a texture is for, which decides both how its pixels are
         * interpreted and whether they need converting first: a glTF image is
         * not always usable as it stands. See make_specular_mask.
         */
        enum class TextureRole
        {
            BASE_COLOR,
            SPECULAR_MASK,
            NORMAL_MAP
        };

        /**
         * Only the base color is a color; the other two are vector and
         * reflectance data, which must not be sRGB-decoded on sampling.
         */
        TextureContent content_of(TextureRole role)
        {
            return role == TextureRole::BASE_COLOR
                       ? TextureContent::COLOR
                       : TextureContent::DATA;
        }

        /**
         * The bytes of a buffer or image, or an empty span for a source the
         * importer cannot reach. Everything is in memory already: the parser
         * runs with LoadExternalBuffers and LoadExternalImages.
         */
        std::span<const std::byte> get_bytes(const fastgltf::DataSource& source)
        {
            if (const auto* array = std::get_if<fastgltf::sources::Array>(&source))
                return {array->bytes.data(), array->bytes.size()};
            if (const auto* vector = std::get_if<fastgltf::sources::Vector>(&source))
                return {vector->bytes.data(), vector->bytes.size()};
            if (const auto* view = std::get_if<fastgltf::sources::ByteView>(&source))
                return {view->bytes.data(), view->bytes.size()};
            return {};
        }

        Xyz::Matrix4F to_matrix(const fastgltf::math::fmat4x4& m)
        {
            std::array<float, 16> values{};
            for (size_t col = 0; col < 4; ++col)
            {
                for (size_t row = 0; row < 4; ++row)
                    values[col * 4 + row] = m[col][row];
            }
            return Detail::to_matrix(values);
        }

        /**
         * The node's local transform. glTF stores either TRS components or a
         * matrix, never both, and Scene can store either exactly — so neither
         * has to be decomposed into the other.
         */
        Xyz::Matrix4F get_local_matrix(const fastgltf::Node& node)
        {
            if (const auto* trs = std::get_if<fastgltf::TRS>(&node.transform))
            {
                const auto& t = trs->translation;
                const auto& r = trs->rotation;
                const auto& s = trs->scale;
                return Transform{
                    .translation = {float(t[0]), float(t[1]), float(t[2])},
                    .rotation = Detail::to_quaternion({float(r[0]), float(r[1]),
                                                       float(r[2]), float(r[3])}),
                    .scale = {float(s[0]), float(s[1]), float(s[2])}
                }.make_matrix();
            }
            return to_matrix(std::get<fastgltf::math::fmat4x4>(node.transform));
        }

        /**
         * The Tungsten topology for a glTF primitive mode, or nothing for the
         * modes this importer will not draw. Points and lines carry no
         * surface to light, and the Blinn-Phong family is all there is.
         */
        std::optional<TopologyType> to_topology(fastgltf::PrimitiveType type)
        {
            switch (type)
            {
            case fastgltf::PrimitiveType::Triangles:
                return TopologyType::TRIANGLES;
            case fastgltf::PrimitiveType::TriangleStrip:
                return TopologyType::TRIANGLE_STRIP;
            case fastgltf::PrimitiveType::TriangleFan:
                return TopologyType::TRIANGLE_FAN;
            default:
                return {};
            }
        }

        const char* to_string(fastgltf::PrimitiveType type)
        {
            switch (type)
            {
            case fastgltf::PrimitiveType::Points: return "POINTS";
            case fastgltf::PrimitiveType::Lines: return "LINES";
            case fastgltf::PrimitiveType::LineLoop: return "LINE_LOOP";
            case fastgltf::PrimitiveType::LineStrip: return "LINE_STRIP";
            case fastgltf::PrimitiveType::Triangles: return "TRIANGLES";
            case fastgltf::PrimitiveType::TriangleStrip: return "TRIANGLE_STRIP";
            case fastgltf::PrimitiveType::TriangleFan: return "TRIANGLE_FAN";
            }
            return "an unknown mode";
        }

        SamplerMinMagFilter to_min_mag_filter(fastgltf::Filter filter)
        {
            switch (filter)
            {
            case fastgltf::Filter::Nearest:
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::NearestMipMapLinear:
                return SamplerMinMagFilter::NEAREST;
            default:
                return SamplerMinMagFilter::LINEAR;
            }
        }

        SamplerAddressMode to_address_mode(fastgltf::Wrap wrap)
        {
            switch (wrap)
            {
            case fastgltf::Wrap::ClampToEdge:
                return SamplerAddressMode::CLAMP_TO_EDGE;
            case fastgltf::Wrap::MirroredRepeat:
                return SamplerAddressMode::MIRRORED_REPEAT;
            default:
                return SamplerAddressMode::REPEAT;
            }
        }

        /**
         * Turns one glTF scene into Tungsten nodes, meshes, materials and
         * textures. One instance per import: it holds the caches that keep a
         * material or an image shared by several primitives from being built
         * more than once.
         */
        class Importer
        {
        public:
            Importer(const fastgltf::Asset& asset,
                     std::filesystem::path directory,
                     ResourceManager& resources,
                     Scene& scene)
                : asset_(asset),
                  directory_(std::move(directory)),
                  resources_(resources),
                  scene_(scene)
            {
                // One layout for the whole file. The Blinn-Phong family
                // requires all three of these, so a primitive that lacks
                // normals or texture coordinates gets them synthesized rather
                // than a layout of its own.
                layout_ = resources_.register_layout(
                    VertexLayoutBuilder()
                        .add_attribute(AttributeSemantic::POSITION)
                        .add_attribute(AttributeSemantic::NORMAL)
                        .add_attribute(AttributeSemantic::TEX_COORD_0)
                        .build());
                shader_ = resources_.register_shader_variant(
                    {.family = BLINN_PHONG_FAMILY});
            }

            GltfImport run(size_t scene_index, NodeId parent)
            {
                result_.root = scene_.create_node(parent);

                for (const auto node_index : asset_.scenes[scene_index].nodeIndices)
                    import_node(node_index, result_.root);

                create_meshes();

                // World matrices are what the bounds have to be measured in,
                // and they are only valid after a resolve.
                scene_.resolve_transforms();
                const auto root_to_world = scene_.world_matrix(result_.root);
                const auto world_to_root = Xyz::invert(root_to_world);
                for (const auto& [id, bounds] : local_bounds_)
                {
                    result_.bounds += Xyz::transform_bbox(
                        bounds, world_to_root * scene_.world_matrix(id));
                }

                return std::move(result_);
            }

        private:
            void import_node(size_t node_index, NodeId parent)
            {
                const auto& node = asset_.nodes[node_index];
                const auto id = scene_.create_node(parent);
                scene_.set_local_transform(id, get_local_matrix(node));

                if (node.skinIndex.has_value())
                {
                    warn("node '" + name_of(node.name, node_index)
                         + "' is skinned; it is drawn in its bind pose because"
                           " skinning is not supported.");
                }

                if (node.meshIndex.has_value())
                    import_mesh(*node.meshIndex, id);

                for (const auto child : node.children)
                    import_node(child, id);
            }

            void import_mesh(size_t mesh_index, NodeId node)
            {
                const auto& mesh = asset_.meshes[mesh_index];
                for (size_t i = 0; i < mesh.primitives.size(); ++i)
                {
                    const auto& primitive = mesh.primitives[i];
                    const auto where = "primitive " + std::to_string(i)
                                       + " of mesh '"
                                       + name_of(mesh.name, mesh_index) + "'";

                    const auto topology = to_topology(primitive.type);
                    if (!topology)
                    {
                        warn("skipped " + where + ": " + to_string(primitive.type)
                             + " has no surface to shade.");
                        continue;
                    }

                    auto data = read_primitive(primitive, where);
                    if (!data)
                        continue;

                    // Each primitive becomes its own node: a renderable is one
                    // mesh and one material, and the primitives of a glTF mesh
                    // have neither in common. The renderable itself waits for
                    // create_meshes, which needs to know the whole file's size
                    // before it can allocate anything.
                    pending_.push_back({
                        .node = scene_.create_node(node),
                        .data = std::move(*data),
                        .material = get_material(primitive, *topology)
                    });
                    ++result_.primitive_count;
                }
            }

            std::optional<PrimitiveData>
            read_primitive(const fastgltf::Primitive& primitive,
                           const std::string& where)
            {
                const auto* position = primitive.findAttribute("POSITION");
                if (position == primitive.attributes.end())
                {
                    warn("skipped " + where + ": it has no POSITION attribute.");
                    return {};
                }

                PrimitiveData data;
                const auto& positions = asset_.accessors[position->accessorIndex];
                data.positions.resize(positions.count);
                fastgltf::copyFromAccessor<Xyz::Vector3F>(
                    asset_, positions, data.positions.data());

                if (const auto* normal = primitive.findAttribute("NORMAL");
                    normal != primitive.attributes.end())
                {
                    const auto& accessor = asset_.accessors[normal->accessorIndex];
                    data.normals.resize(accessor.count);
                    fastgltf::copyFromAccessor<Xyz::Vector3F>(
                        asset_, accessor, data.normals.data());
                }

                if (const auto* uv = primitive.findAttribute("TEXCOORD_0");
                    uv != primitive.attributes.end())
                {
                    const auto& accessor = asset_.accessors[uv->accessorIndex];
                    data.tex_coords.resize(accessor.count);
                    fastgltf::copyFromAccessor<Xyz::Vector2F>(
                        asset_, accessor, data.tex_coords.data());
                }

                // GenerateMeshIndices means the parser has already given an
                // unindexed primitive a trivial index buffer.
                if (!primitive.indicesAccessor.has_value())
                {
                    warn("skipped " + where + ": it has no indices.");
                    return {};
                }
                const auto& indices = asset_.accessors[*primitive.indicesAccessor];
                data.indices.resize(indices.count);
                fastgltf::copyFromAccessor<uint32_t>(
                    asset_, indices, data.indices.data());

                Detail::complete_primitive(data);
                if (data.positions.empty() || data.indices.empty())
                {
                    warn("skipped " + where + ": it has no triangles.");
                    return {};
                }
                return data;
            }

            /**
             * Uploads every primitive read so far and gives its node a
             * renderable. Deferred to here so that the arenas can be sized
             * once, for the whole file.
             */
            void create_meshes()
            {
                if (pending_.empty())
                    return;

                std::vector<uint32_t> vertex_sizes;
                std::vector<uint32_t> index_sizes;
                vertex_sizes.reserve(pending_.size());
                index_sizes.reserve(pending_.size());
                for (const auto& primitive : pending_)
                {
                    vertex_sizes.push_back(uint32_t(primitive.data.positions.size()));
                    index_sizes.push_back(uint32_t(primitive.data.indices.size()));
                }

                vbo_arena_ = resources_.create_arena(
                    BufferUsage::STATIC_DRAW,
                    GLTF_VERTEX_STRIDE * sizeof(float),
                    required_capacity(vertex_sizes));
                ebo_arena_ = resources_.create_arena(
                    BufferUsage::STATIC_DRAW, sizeof(uint32_t),
                    required_capacity(index_sizes));

                for (auto& primitive : pending_)
                {
                    const auto bounds = Detail::make_bounds(primitive.data.positions);
                    scene_.add_component(primitive.node, RenderableComponent{
                        .mesh = create_mesh(primitive.data),
                        .material = primitive.material,
                        .local_bounds = bounds
                    });
                    local_bounds_.emplace_back(primitive.node, bounds);
                }
                pending_.clear();
            }

            MeshRef create_mesh(const PrimitiveData& data)
            {
                const auto vertex_data = Detail::interleave(data);
                const auto vertices = resources_.allocate(
                    vbo_arena_, uint32_t(data.positions.size()));
                const auto indices = resources_.allocate(
                    ebo_arena_, uint32_t(data.indices.size()));

                // Rebase to absolute indices: there is no baseVertex draw
                // argument, and every primitive in the file shares one arena,
                // so these offsets leave 16-bit indices behind immediately.
                auto index_data = data.indices;
                for (auto& index : index_data)
                    index += vertices.offset;

                resources_.upload(vertices, vertex_data.data(),
                                  vertex_data.size() * sizeof(float));
                resources_.upload(indices, index_data.data(),
                                  index_data.size() * sizeof(uint32_t));

                Mesh mesh;
                mesh.streams = {vertices};
                mesh.layout = layout_;
                mesh.ebo = indices;
                mesh.index_type = ElementIndexType::UINT32;
                return resources_.create_mesh(std::move(mesh));
            }

            MaterialRef get_material(const fastgltf::Primitive& primitive,
                                     TopologyType topology)
            {
                const auto index = primitive.materialIndex.has_value()
                                       ? *primitive.materialIndex
                                       : NO_MATERIAL;
                const auto pipeline = get_pipeline(index, topology);
                const MaterialKey key{index, pipeline.index, pipeline.generation};
                if (const auto it = materials_.find(key); it != materials_.end())
                    return it->second;

                Material material;
                material.pipeline = pipeline;
                if (index == NO_MATERIAL)
                {
                    // glTF's default material is metallic and fully rough,
                    // which Blinn-Phong renders as very nearly black. A
                    // neutral surface is a more useful stand-in.
                    material.parameter_data = make_blinn_phong_material_params(
                        ColorMaterial{});
                }
                else
                {
                    const auto& source = asset_.materials[index];
                    const auto& pbr = source.pbrData;
                    const auto& base = pbr.baseColorFactor;
                    const auto color_material = Detail::to_color_material(
                        {float(base[0]), float(base[1]), float(base[2]),
                         float(base[3])},
                        float(pbr.metallicFactor), float(pbr.roughnessFactor));

                    const auto normal_strength =
                        source.normalTexture.has_value()
                            ? float(source.normalTexture->scale)
                            : 0.0f;
                    material.parameter_data = make_blinn_phong_material_params(
                        color_material, float(base[3]), normal_strength);
                    material.textures = get_textures(source);
                }
                const auto ref = resources_.create_material(std::move(material));
                materials_.emplace(key, ref);
                return ref;
            }

            /**
             * The material's textures in the slot order the Blinn-Phong family
             * fixes: diffuse, specular, normal. A null ref gets the renderer's
             * white fallback, so an earlier slot can be left empty to reach a
             * later one. Trailing nulls are dropped.
             */
            std::vector<TextureRef> get_textures(const fastgltf::Material& source)
            {
                std::vector<TextureRef> textures(3);
                if (source.pbrData.baseColorTexture)
                {
                    textures[0] = get_texture(
                        source.pbrData.baseColorTexture->textureIndex,
                        TextureRole::BASE_COLOR);
                }
                if (source.pbrData.metallicRoughnessTexture)
                {
                    textures[1] = get_texture(
                        source.pbrData.metallicRoughnessTexture->textureIndex,
                        TextureRole::SPECULAR_MASK);
                }
                if (source.normalTexture)
                {
                    textures[2] = get_texture(source.normalTexture->textureIndex,
                                              TextureRole::NORMAL_MAP);
                }
                while (!textures.empty() && !textures.back())
                    textures.pop_back();
                return textures;
            }

            TextureRef get_texture(size_t index, TextureRole role)
            {
                const TextureKey key{index, role};
                if (const auto it = textures_.find(key); it != textures_.end())
                    return it->second;

                TextureRef ref;
                const auto& texture = asset_.textures[index];
                if (!texture.imageIndex.has_value())
                {
                    warn("texture " + std::to_string(index)
                         + " has no image this build can read; it is drawn"
                           " without one.");
                }
                else if (const auto image = decode_image(*texture.imageIndex))
                {
                    // Yimage measures a pixel in bits, and supports types with
                    // fewer than eight of them.
                    const auto bytes_per_pixel =
                        Yimage::get_pixel_size(image->pixel_type()) / 8;

                    try
                    {
                        if (bytes_per_pixel == 0)
                        {
                            TUNGSTEN_THROW("sub-byte pixels are not supported");
                        }

                        // A metallic-roughness image has to be folded down to
                        // the one greyscale channel u_specular_map can use
                        // before it goes anywhere near the GPU.
                        std::vector<uint8_t> mask;
                        if (role == TextureRole::SPECULAR_MASK)
                        {
                            mask = Detail::make_specular_mask(
                                image->data(),
                                image->width() * image->height(),
                                bytes_per_pixel);
                        }

                        ref = resources_.create_texture({
                            .size = {int32_t(image->width()),
                                     int32_t(image->height())},
                            .format = mask.empty()
                                          ? get_ogl_pixel_type(image->pixel_type())
                                          : RGB_TEXTURE,
                            .content = content_of(role),
                            .pixels = mask.empty() ? image->data() : mask.data(),
                            .sampler = get_sampler(texture.samplerIndex)
                        });
                    }
                    catch (const std::exception& e)
                    {
                        // A texture Tungsten has no format for is a reason to
                        // draw the model without it, not to refuse the file.
                        warn("texture " + std::to_string(index) + " could not"
                             " be uploaded (" + e.what() + "); it is drawn"
                             " without it.");
                    }
                }
                textures_.emplace(key, ref);
                return ref;
            }

            std::optional<Yimage::Image> decode_image(size_t index)
            {
                const auto& image = asset_.images[index];
                auto bytes = get_bytes(image.data);

                if (const auto* view =
                    std::get_if<fastgltf::sources::BufferView>(&image.data))
                {
                    const auto& buffer_view = asset_.bufferViews[view->bufferViewIndex];
                    const auto buffer = get_bytes(
                        asset_.buffers[buffer_view.bufferIndex].data);
                    if (buffer.size() >= buffer_view.byteOffset + buffer_view.byteLength)
                    {
                        bytes = buffer.subspan(buffer_view.byteOffset,
                                               buffer_view.byteLength);
                    }
                }

                try
                {
                    if (!bytes.empty())
                        return read_image(bytes.data(), bytes.size());

                    if (const auto* uri =
                        std::get_if<fastgltf::sources::URI>(&image.data))
                    {
                        if (uri->uri.isLocalPath())
                        {
                            return read_image(
                                (directory_ / uri->uri.fspath()).string());
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    warn("image " + name_of(image.name, index) + " could not be"
                         " decoded (" + e.what() + "); it is drawn without it.");
                    return {};
                }

                warn("image " + name_of(image.name, index) + " is stored in a way"
                     " this importer cannot read; it is drawn without it.");
                return {};
            }

            SamplerRef get_sampler(fastgltf::Optional<size_t> index)
            {
                SamplerDescriptor descriptor;
                // No mipmaps are generated for imported textures, so asking
                // for a mip filter would sample a level that does not exist.
                descriptor.mip_filter = SamplerMipFilter::NONE;
                if (index.has_value())
                {
                    const auto& sampler = asset_.samplers[*index];
                    if (sampler.minFilter.has_value())
                        descriptor.min_filter = to_min_mag_filter(*sampler.minFilter);
                    if (sampler.magFilter.has_value())
                        descriptor.mag_filter = to_min_mag_filter(*sampler.magFilter);
                    descriptor.address_mode_u = to_address_mode(sampler.wrapS);
                    descriptor.address_mode_v = to_address_mode(sampler.wrapT);
                }
                else
                {
                    descriptor.min_filter = SamplerMinMagFilter::LINEAR;
                }
                return resources_.register_sampler(descriptor);
            }

            PipelineRef get_pipeline(size_t material_index, TopologyType topology)
            {
                PipelineDescriptor descriptor;
                descriptor.shader = shader_;
                descriptor.layout = layout_;
                descriptor.primitive = topology;
                descriptor.raster.cull_enabled = true;

                if (material_index != NO_MATERIAL)
                {
                    const auto& material = asset_.materials[material_index];
                    descriptor.raster.cull_enabled = !material.doubleSided;
                    if (material.alphaMode == fastgltf::AlphaMode::Blend)
                    {
                        descriptor.queue = RenderQueue::TRANSPARENT;
                        descriptor.depth.write = false;
                        descriptor.blend = {
                            .enabled = true,
                            .src_color = BlendFunction::SRC_ALPHA,
                            .dst_color = BlendFunction::ONE_MINUS_SRC_ALPHA,
                            .src_alpha = BlendFunction::SRC_ALPHA,
                            .dst_alpha = BlendFunction::ONE_MINUS_SRC_ALPHA
                        };
                    }
                    else if (material.alphaMode == fastgltf::AlphaMode::Mask
                             && !warned_about_alpha_mask_)
                    {
                        warned_about_alpha_mask_ = true;
                        warn("alpha masking is not supported; masked materials"
                             " are drawn fully opaque.");
                    }
                }
                // Interned, so building a descriptor per primitive is free.
                return resources_.register_pipeline(descriptor);
            }

            void warn(std::string message)
            {
                result_.warnings.push_back(std::move(message));
            }

            static std::string name_of(std::string_view name, size_t index)
            {
                return name.empty() ? std::to_string(index) : std::string(name);
            }

            struct PendingPrimitive
            {
                NodeId node;
                PrimitiveData data;
                MaterialRef material;
            };

            static constexpr size_t NO_MATERIAL = size_t(-1);

            using MaterialKey = std::tuple<size_t, uint32_t, uint32_t>;
            using TextureKey = std::pair<size_t, TextureRole>;

            const fastgltf::Asset& asset_;
            std::filesystem::path directory_;
            ResourceManager& resources_;
            Scene& scene_;
            GltfImport result_;
            VertexLayoutRef layout_;
            ShaderProgramRef shader_;
            BufferArenaRef vbo_arena_;
            BufferArenaRef ebo_arena_;
            std::map<MaterialKey, MaterialRef> materials_;
            std::map<TextureKey, TextureRef> textures_;
            std::vector<PendingPrimitive> pending_;
            std::vector<std::pair<NodeId, Xyz::BBox3F>> local_bounds_;
            bool warned_about_alpha_mask_ = false;
        };
    }

    GltfImport import_gltf(const std::filesystem::path& file_name,
                           ResourceManager& resources,
                           Scene& scene,
                           NodeId parent,
                           const GltfImportOptions& options)
    {
        // fastgltf reports a missing file as an invalid *directory*, which
        // sends the reader looking in the wrong place.
        if (!std::filesystem::is_regular_file(file_name))
        {
            TUNGSTEN_THROW("" + file_name.string()
                           + ": no such file.");
        }

        auto data = fastgltf::GltfDataBuffer::FromPath(file_name);
        if (data.error() != fastgltf::Error::None)
        {
            TUNGSTEN_THROW("" + file_name.string() + ": "
                           + std::string(fastgltf::getErrorMessage(data.error())));
        }

        fastgltf::Parser parser;
        auto asset = parser.loadGltf(
            data.get(), file_name.parent_path(),
            fastgltf::Options::LoadExternalBuffers
            | fastgltf::Options::LoadExternalImages
            | fastgltf::Options::GenerateMeshIndices);
        if (asset.error() != fastgltf::Error::None)
        {
            TUNGSTEN_THROW("" + file_name.string() + ": "
                           + std::string(fastgltf::getErrorMessage(asset.error())));
        }

        const auto scene_index = options.scene_index
                                     ? *options.scene_index
                                     : asset->defaultScene.value_or(0);
        if (scene_index >= asset->scenes.size())
        {
            TUNGSTEN_THROW("" + file_name.string() + ": the file has no scene "
                           + std::to_string(scene_index) + ".");
        }

        Importer importer(asset.get(), file_name.parent_path(), resources, scene);
        auto result = importer.run(scene_index, parent);

        if (result.primitive_count == 0)
        {
            scene.remove(result.root);
            TUNGSTEN_THROW("" + file_name.string() + ": the scene contains"
                           " nothing that can be displayed. Only triangle"
                           " geometry is supported."
                           + describe(result.warnings));
        }
        if (!result.bounds)
        {
            scene.remove(result.root);
            TUNGSTEN_THROW("" + file_name.string() + ": the scene's geometry"
                           " has no extent, so there is nothing to look at."
                           + describe(result.warnings));
        }

        return result;
    }
} // Tungsten
