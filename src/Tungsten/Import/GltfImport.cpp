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
#include <optional>
#include <string>
#include <utility>
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
#include "Tungsten/SceneGraph/Skin.hpp"
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

template <>
struct fastgltf::ElementTraits<Xyz::Vector4F>
    : fastgltf::ElementTraitsBase<Xyz::Vector4F, fastgltf::AccessorType::Vec4, float>
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
            // A lone allocation lands at offset 0 and needs no rounding.
            if (sizes.size() == 1)
                return std::max(sizes[0], 1u);

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
         * Undoes a failed import: removes its nodes from the scene and its
         * resources from the manager.
         */
        void discard(GltfImport& import, Scene& scene, ResourceManager& resources)
        {
            if (import.root)
                scene.remove(import.root);
            resources.destroy_bundle(import.resources);
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

        Transform to_transform(const fastgltf::TRS& trs)
        {
            const auto& t = trs.translation;
            const auto& r = trs.rotation;
            const auto& s = trs.scale;
            return {
                .translation = {float(t[0]), float(t[1]), float(t[2])},
                .rotation = Detail::to_quaternion({float(r[0]), float(r[1]),
                                                   float(r[2]), float(r[3])}),
                .scale = {float(s[0]), float(s[1]), float(s[2])}
            };
        }

        /**
         * The node's local transform. glTF stores either TRS components or a
         * matrix, never both, and Scene can store either exactly — so neither
         * has to be decomposed into the other.
         */
        Xyz::Matrix4F get_local_matrix(const fastgltf::Node& node)
        {
            if (const auto* trs = std::get_if<fastgltf::TRS>(&node.transform))
                return to_transform(*trs).make_matrix();
            return to_matrix(std::get<fastgltf::math::fmat4x4>(node.transform));
        }

        Interpolation to_interpolation(fastgltf::AnimationInterpolation value)
        {
            switch (value)
            {
            case fastgltf::AnimationInterpolation::Step:
                return Interpolation::STEP;
            case fastgltf::AnimationInterpolation::CubicSpline:
                return Interpolation::CUBIC_SPLINE;
            default:
                return Interpolation::LINEAR;
            }
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
        class GltfImporter
        {
        public:
            GltfImporter(const fastgltf::Asset& asset,
                     std::filesystem::path directory,
                     ResourceManager& resources,
                     Scene& scene,
                     const GltfImportOptions& options)
                : asset_(asset),
                  directory_(std::move(directory)),
                  resources_(resources),
                  scene_(scene),
                  create_wireframes_(options.create_wireframes)
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
                try
                {
                    result_.root = scene_.create_node(parent);
                    result_.targets.nodes.resize(asset_.nodes.size());
                    result_.targets.rest_transforms.resize(asset_.nodes.size());

                    prepare_skins(scene_index);
                    for (const auto node_index : asset_.scenes[scene_index].nodeIndices)
                        import_node(node_index, result_.root);

                    create_meshes();
                    import_animations();
                }
                catch (...)
                {
                    discard(result_, scene_, resources_);
                    throw;
                }

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
                result_.targets.nodes[node_index] = id;
                if (const auto* trs = std::get_if<fastgltf::TRS>(&node.transform))
                    result_.targets.rest_transforms[node_index] = to_transform(*trs);

                if (node.meshIndex.has_value())
                {
                    // A skin prepare_skins rejected leaves the mesh rigid; it
                    // has already said why.
                    std::optional<size_t> skin;
                    if (node.skinIndex.has_value()
                        && inverse_bind_matrices_.contains(*node.skinIndex))
                    {
                        skin = *node.skinIndex;
                    }
                    import_mesh(*node.meshIndex, id, skin);
                }

                for (const auto child : node.children)
                    import_node(child, id);
            }

            /**
             * Reads the inverse bind matrices of every skin the imported
             * scene can be drawn with: one whose joints are all in that
             * scene, and that has no more of them than the skinned shader
             * takes. The meshes of any other skin are drawn rigid, in their
             * bind pose, with a warning.
             */
            void prepare_skins(size_t scene_index)
            {
                if (asset_.skins.empty())
                    return;

                std::vector<uint8_t> in_scene(asset_.nodes.size());
                std::vector<size_t> pending(
                    asset_.scenes[scene_index].nodeIndices.begin(),
                    asset_.scenes[scene_index].nodeIndices.end());
                while (!pending.empty())
                {
                    const auto index = pending.back();
                    pending.pop_back();
                    in_scene[index] = 1;
                    for (const auto child : asset_.nodes[index].children)
                        pending.push_back(child);
                }

                for (size_t i = 0; i < asset_.skins.size(); ++i)
                {
                    const auto& skin = asset_.skins[i];
                    const auto what = "skin '" + name_of(skin.name, i) + "'";
                    const auto fallback = "; its meshes are drawn in their bind"
                                          " pose.";
                    if (skin.joints.size() > MAX_SKIN_JOINTS)
                    {
                        warn(what + " has " + std::to_string(skin.joints.size())
                             + " joints, more than the "
                             + std::to_string(MAX_SKIN_JOINTS)
                             + " supported" + fallback);
                        continue;
                    }
                    const auto outside = std::ranges::any_of(
                        skin.joints, [&](size_t joint) { return !in_scene[joint]; });
                    if (outside)
                    {
                        warn(what + " has joints outside the imported scene"
                             + fallback);
                        continue;
                    }

                    try
                    {
                        inverse_bind_matrices_.emplace(
                            i, read_inverse_bind_matrices(skin));
                    }
                    catch (const std::exception& e)
                    {
                        warn(what + " could not be read (" + e.what() + ")"
                             + fallback);
                    }
                }
            }

            /**
             * The skin's inverse bind matrices, one per joint. glTF makes
             * them optional, and identities when they are left out.
             */
            std::vector<Xyz::Matrix4F>
            read_inverse_bind_matrices(const fastgltf::Skin& skin)
            {
                std::vector<Xyz::Matrix4F> result;
                if (!skin.inverseBindMatrices.has_value())
                {
                    result.assign(skin.joints.size(), Xyz::Matrix4F::identity());
                    return result;
                }

                const auto matrices = read_accessor<fastgltf::math::fmat4x4>(
                    *skin.inverseBindMatrices);
                if (matrices.size() < skin.joints.size())
                    TUNGSTEN_THROW("it has fewer inverse bind matrices than joints");
                result.reserve(skin.joints.size());
                for (size_t i = 0; i < skin.joints.size(); ++i)
                    result.push_back(to_matrix(matrices[i]));
                return result;
            }

            /**
             * The skin the renderables of glTF skin @a index share. Built on
             * first use, which comes after every node has been created: a
             * skin can name joints the scene reaches after its mesh.
             */
            std::shared_ptr<const Skin> get_skin(size_t index)
            {
                if (const auto it = skins_.find(index); it != skins_.end())
                    return it->second;

                Skin skin;
                skin.inverse_bind_matrices = inverse_bind_matrices_.at(index);
                for (const auto joint : asset_.skins[index].joints)
                    skin.joints.push_back(result_.targets.nodes[joint]);
                auto result = std::make_shared<const Skin>(std::move(skin));
                skins_.emplace(index, result);
                return result;
            }

            void import_mesh(size_t mesh_index, NodeId node,
                             std::optional<size_t> skin)
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

                    auto data = read_primitive(primitive, where, skin);
                    if (!data)
                        continue;
                    const auto skinned = !data->joints.empty();

                    // Each primitive becomes its own node: a renderable is one
                    // mesh and one material, and the primitives of a glTF mesh
                    // have neither in common. The renderable itself waits for
                    // create_meshes, which needs to know the whole file's size
                    // before it can allocate anything.
                    PendingPrimitive pending{
                        .node = scene_.create_node(node),
                        .material = get_material(primitive, *topology, skinned)
                    };
                    if (skinned)
                        pending.skin = skin;
                    if (create_wireframes_)
                    {
                        // The same material through a line pipeline, so the
                        // wireframe keeps the surface's colors and lighting.
                        pending.line_indices = Detail::make_line_indices(
                            data->indices, *topology);
                        pending.line_material = get_material(
                            primitive, TopologyType::LINES, skinned);
                    }
                    pending.data = std::move(*data);
                    pending_.push_back(std::move(pending));
                    ++result_.primitive_count;
                }
            }

            /**
             * Turns the file's animations into clips. An animation with a
             * channel that cannot be read is skipped as a whole: playing the
             * rest of it would show a motion the file does not describe.
             */
            void import_animations()
            {
                for (size_t i = 0; i < asset_.animations.size(); ++i)
                {
                    const auto name = name_of(asset_.animations[i].name, i);
                    try
                    {
                        auto channels = read_channels(asset_.animations[i], name);
                        if (channels.empty())
                            continue;
                        result_.animations.push_back(
                            std::make_shared<const AnimationClip>(
                                make_animation_clip(name, std::move(channels))));
                    }
                    catch (const std::exception& e)
                    {
                        warn("skipped animation '" + name + "': " + e.what());
                    }
                }
            }

            std::vector<AnimationChannel>
            read_channels(const fastgltf::Animation& animation,
                          const std::string& name)
            {
                std::vector<AnimationChannel> channels;
                bool warned_about_weights = false;
                for (const auto& channel : animation.channels)
                {
                    // A channel without a node targets something an
                    // extension defines.
                    if (!channel.nodeIndex.has_value())
                        continue;
                    // Animations belong to the file, not to a scene, so this
                    // can be a node of a scene that was not imported.
                    const auto node_index = *channel.nodeIndex;
                    if (!result_.targets.nodes[node_index])
                        continue;

                    if (channel.path == fastgltf::AnimationPath::Weights)
                    {
                        if (!std::exchange(warned_about_weights, true))
                        {
                            warn("animation '" + name + "' animates morph"
                                 " target weights, which are not supported;"
                                 " those channels are skipped.");
                        }
                        continue;
                    }

                    const auto& node = asset_.nodes[node_index];
                    if (!std::holds_alternative<fastgltf::TRS>(node.transform))
                    {
                        warn("animation '" + name + "' animates node '"
                             + name_of(node.name, node_index) + "', which is"
                             " given as a matrix; that channel is skipped.");
                        continue;
                    }

                    const auto& sampler = animation.samplers[channel.samplerIndex];
                    const auto interpolation = to_interpolation(sampler.interpolation);
                    auto times = read_accessor<float>(sampler.inputAccessor);
                    AnimationChannel result{.target = uint32_t(node_index)};
                    switch (channel.path)
                    {
                    case fastgltf::AnimationPath::Translation:
                        result.path = AnimationPath::TRANSLATION;
                        result.track = Detail::make_vector3_track(
                            interpolation, std::move(times),
                            read_accessor<Xyz::Vector3F>(sampler.outputAccessor));
                        break;
                    case fastgltf::AnimationPath::Rotation:
                        result.path = AnimationPath::ROTATION;
                        result.track = Detail::make_rotation_track(
                            interpolation, std::move(times),
                            read_accessor<Xyz::Vector4F>(sampler.outputAccessor));
                        break;
                    case fastgltf::AnimationPath::Scale:
                        result.path = AnimationPath::SCALE;
                        result.track = Detail::make_vector3_track(
                            interpolation, std::move(times),
                            read_accessor<Xyz::Vector3F>(sampler.outputAccessor));
                        break;
                    default:
                        continue;
                    }
                    channels.push_back(std::move(result));
                }
                return channels;
            }

            /**
             * Every element of an accessor, widened to T. Throws if the
             * accessor's type is not T's, which copyFromAccessor does not
             * check.
             */
            template <typename T>
            std::vector<T> read_accessor(size_t index)
            {
                const auto& accessor = asset_.accessors[index];
                if (accessor.type != fastgltf::ElementTraits<T>::type)
                {
                    TUNGSTEN_THROW("accessor " + std::to_string(index)
                                   + " has the wrong type.");
                }
                std::vector<T> result(accessor.count);
                fastgltf::copyFromAccessor<T>(asset_, accessor, result.data());
                return result;
            }

            std::optional<PrimitiveData>
            read_primitive(const fastgltf::Primitive& primitive,
                           const std::string& where,
                           std::optional<size_t> skin)
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

                if (skin)
                    read_skinning(primitive, *skin, where, data);

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
             * Adds the primitive's joints and weights to @a data. A primitive
             * without usable ones stays rigid, and is drawn in its bind pose
             * with a warning.
             */
            void read_skinning(const fastgltf::Primitive& primitive,
                               size_t skin, const std::string& where,
                               PrimitiveData& data)
            {
                const auto fallback = "; it is drawn in its bind pose.";
                const auto* joints = primitive.findAttribute("JOINTS_0");
                const auto* weights = primitive.findAttribute("WEIGHTS_0");
                if (joints == primitive.attributes.end()
                    || weights == primitive.attributes.end())
                {
                    warn(where + " is skinned, but has no JOINTS_0 or WEIGHTS_0"
                         + fallback);
                    return;
                }

                std::vector<Xyz::Vector4F> joint_data;
                std::vector<Xyz::Vector4F> weight_data;
                try
                {
                    joint_data = read_accessor<Xyz::Vector4F>(joints->accessorIndex);
                    weight_data = read_accessor<Xyz::Vector4F>(weights->accessorIndex);
                }
                catch (const std::exception& e)
                {
                    warn(where + "'s joints or weights could not be read ("
                         + e.what() + ")" + fallback);
                    return;
                }

                if (joint_data.size() != data.positions.size()
                    || weight_data.size() != data.positions.size())
                {
                    warn(where + " has a different number of joints, weights"
                         " and positions" + fallback);
                    return;
                }
                if (!Detail::are_valid_joint_indices(
                        joint_data, asset_.skins[skin].joints.size()))
                {
                    warn(where + " has joint indices outside its skin"
                         + fallback);
                    return;
                }
                if (primitive.findAttribute("JOINTS_1") != primitive.attributes.end())
                {
                    warn(where + " has more than four joints per vertex; only"
                         " the first four are used.");
                }

                // Normalizing also makes up for any influences dropped above.
                Detail::normalize_weights(weight_data);
                data.joints = std::move(joint_data);
                data.weights = std::move(weight_data);
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

                // Rigid and skinned vertices differ in size, and an arena
                // holds one size of vertex.
                std::vector<uint32_t> vertex_sizes;
                std::vector<uint32_t> skinned_vertex_sizes;
                std::vector<uint32_t> index_sizes;
                index_sizes.reserve(pending_.size());
                for (const auto& primitive : pending_)
                {
                    auto& sizes = primitive.skin ? skinned_vertex_sizes
                                                 : vertex_sizes;
                    sizes.push_back(uint32_t(primitive.data.positions.size()));
                    index_sizes.push_back(uint32_t(primitive.data.indices.size()));
                    if (!primitive.line_indices.empty())
                        index_sizes.push_back(uint32_t(primitive.line_indices.size()));
                }

                // The file's own arenas, so its slices are freed with them
                // and need not be listed in the bundle.
                if (!vertex_sizes.empty())
                {
                    vbo_arena_ = resources_.create_arena(
                        BufferUsage::STATIC_DRAW,
                        GLTF_VERTEX_STRIDE * sizeof(float),
                        required_capacity(vertex_sizes));
                    result_.resources.arenas.push_back(vbo_arena_);
                }
                if (!skinned_vertex_sizes.empty())
                {
                    skinned_vbo_arena_ = resources_.create_arena(
                        BufferUsage::STATIC_DRAW,
                        Detail::GLTF_SKINNED_VERTEX_STRIDE * sizeof(float),
                        required_capacity(skinned_vertex_sizes));
                    result_.resources.arenas.push_back(skinned_vbo_arena_);
                }
                ebo_arena_ = resources_.create_arena(
                    BufferUsage::STATIC_DRAW, sizeof(uint32_t),
                    required_capacity(index_sizes));
                result_.resources.arenas.push_back(ebo_arena_);

                for (auto& primitive : pending_)
                {
                    const auto bounds = Detail::make_bounds(primitive.data.positions);
                    const auto vertices = upload_vertices(primitive.data);
                    const auto layout = primitive.skin ? skinned_layout_ : layout_;
                    const auto skin = primitive.skin ? get_skin(*primitive.skin)
                                                     : nullptr;
                    scene_.add_component(primitive.node, RenderableComponent{
                        .mesh = create_mesh(vertices, primitive.data.indices,
                                            layout),
                        .material = primitive.material,
                        // The bind-pose bounds say little about where the
                        // joints will take a skinned mesh, so it is never
                        // culled. They still frame the import as a whole.
                        .local_bounds = skin ? Xyz::BBox3F() : bounds,
                        .skin = skin
                    });
                    local_bounds_.emplace_back(primitive.node, bounds);

                    // A primitive whose triangles are all degenerate has no
                    // edges, and a mesh cannot be empty.
                    if (!primitive.line_indices.empty())
                    {
                        result_.wireframes.push_back({
                            .node = primitive.node,
                            .mesh = create_mesh(vertices, primitive.line_indices,
                                                layout),
                            .material = primitive.line_material
                        });
                    }
                }
                pending_.clear();
            }

            SharedBuffer upload_vertices(const PrimitiveData& data)
            {
                const auto vertex_data = Detail::interleave(data);
                const auto arena = data.joints.empty() ? vbo_arena_
                                                       : skinned_vbo_arena_;
                const auto vertices = resources_.allocate(
                    arena, uint32_t(data.positions.size()));
                resources_.upload(vertices, vertex_data.data(),
                                  vertex_data.size() * sizeof(float));
                return vertices;
            }

            /**
             * Creates a mesh that draws @a vertices through @a indices, which
             * are relative to the first of those vertices.
             */
            MeshRef create_mesh(const SharedBuffer& vertices,
                                const std::vector<uint32_t>& indices,
                                VertexLayoutRef layout)
            {
                const auto ebo = resources_.allocate(
                    ebo_arena_, uint32_t(indices.size()));

                // Rebase to absolute indices: there is no baseVertex draw
                // argument, and every primitive in the file shares one arena,
                // so these offsets leave 16-bit indices behind immediately.
                auto index_data = indices;
                for (auto& index : index_data)
                    index += vertices.offset;

                resources_.upload(ebo, index_data.data(),
                                  index_data.size() * sizeof(uint32_t));

                Mesh mesh;
                mesh.streams = {vertices};
                mesh.layout = layout;
                mesh.ebo = ebo;
                mesh.index_type = ElementIndexType::UINT32;
                const auto ref = resources_.create_mesh(std::move(mesh));
                result_.resources.meshes.push_back(ref);
                return ref;
            }

            MaterialRef get_material(const fastgltf::Primitive& primitive,
                                     TopologyType topology, bool skinned)
            {
                const auto index = primitive.materialIndex.has_value()
                                       ? *primitive.materialIndex
                                       : NO_MATERIAL;
                const auto pipeline = get_pipeline(index, topology, skinned);
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
                result_.resources.materials.push_back(ref);
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
                if (ref)
                    result_.resources.textures.push_back(ref);
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

            PipelineRef get_pipeline(size_t material_index, TopologyType topology,
                                     bool skinned)
            {
                PipelineDescriptor descriptor;
                if (skinned)
                {
                    prepare_skinned_shader();
                    descriptor.shader = skinned_shader_;
                    descriptor.layout = skinned_layout_;
                }
                else
                {
                    descriptor.shader = shader_;
                    descriptor.layout = layout_;
                }
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

            /**
             * Registers the skinned layout and compiles the skinned shader
             * variant, on first use: a file without skins needs neither.
             */
            void prepare_skinned_shader()
            {
                if (skinned_shader_)
                    return;
                skinned_layout_ = resources_.register_layout(
                    VertexLayoutBuilder()
                        .add_attribute(AttributeSemantic::POSITION)
                        .add_attribute(AttributeSemantic::NORMAL)
                        .add_attribute(AttributeSemantic::TEX_COORD_0)
                        .add_attribute(AttributeSemantic::BONE_INDICES)
                        .add_attribute(AttributeSemantic::BONE_WEIGHTS)
                        .build());
                skinned_shader_ = resources_.register_shader_variant(
                    {.family = BLINN_PHONG_FAMILY,
                     .defines = BLINN_PHONG_SKINNED});
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
                MaterialRef material;
                PrimitiveData data;
                std::vector<uint32_t> line_indices;
                MaterialRef line_material;
                /** The glTF skin, for a primitive that is skinned. */
                std::optional<size_t> skin;
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
            VertexLayoutRef skinned_layout_;
            ShaderProgramRef skinned_shader_;
            BufferArenaRef vbo_arena_;
            BufferArenaRef skinned_vbo_arena_;
            BufferArenaRef ebo_arena_;
            std::map<MaterialKey, MaterialRef> materials_;
            std::map<TextureKey, TextureRef> textures_;
            /** Per usable glTF skin, its inverse bind matrices. */
            std::map<size_t, std::vector<Xyz::Matrix4F>> inverse_bind_matrices_;
            std::map<size_t, std::shared_ptr<const Skin>> skins_;
            std::vector<PendingPrimitive> pending_;
            std::vector<std::pair<NodeId, Xyz::BBox3F>> local_bounds_;
            bool create_wireframes_ = false;
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

        GltfImporter importer(asset.get(), file_name.parent_path(), resources, scene,
                          options);
        auto result = importer.run(scene_index, parent);

        if (result.primitive_count == 0)
        {
            discard(result, scene, resources);
            TUNGSTEN_THROW("" + file_name.string() + ": the scene contains"
                           " nothing that can be displayed. Only triangle"
                           " geometry is supported."
                           + describe(result.warnings));
        }
        if (!result.bounds)
        {
            discard(result, scene, resources);
            TUNGSTEN_THROW("" + file_name.string() + ": the scene's geometry"
                           " has no extent, so there is nothing to look at."
                           + describe(result.warnings));
        }

        return result;
    }
} // Tungsten
