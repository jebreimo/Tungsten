//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Import/ObjImport.hpp"

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <tiny_obj_loader.h>
#include "Tungsten/Color.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/YimageGl.hpp"
#include "Tungsten/Resources/BuiltinShaders.hpp"
#include "Tungsten/Model/ModelBuilder.hpp"
#include "Tungsten/Resources/PipelineDescriptor.hpp"
#include "Tungsten/Resources/VertexLayoutBuilder.hpp"
// The primitive helpers are glTF's by name only: they work on plain vertex
// arrays, and the OBJ importer emits the same vertex format.
#include "GltfConversion.hpp"

namespace Tungsten
{
    namespace
    {
        using Detail::PrimitiveData;

        /** The share of the diffuse color that becomes the ambient color, as
         *  for glTF materials. */
        constexpr float AMBIENT_FRACTION = 0.15f;

        /** The range of Ns the Blinn-Phong shader can represent. MTL allows
         *  up to 1000, and 0, which would light every pixel fully. */
        constexpr float MIN_SHININESS = 1.0f;
        constexpr float MAX_SHININESS = 256.0f;

        /** The material id tinyobjloader gives faces that use no material. */
        constexpr int NO_MATERIAL = -1;

        std::string describe(const std::vector<std::string>& warnings)
        {
            std::string result;
            for (const auto& warning : warnings)
                result += "\n  - " + warning;
            return result;
        }

        /**
         * What a texture is for. BUMP_MAP is a map_Bump image, which is used
         * as a normal map unless it turns out to be a greyscale height map.
         */
        enum class TextureRole
        {
            DIFFUSE,
            SPECULAR,
            NORMAL_MAP,
            BUMP_MAP
        };

        TextureContent content_of(TextureRole role)
        {
            return role == TextureRole::DIFFUSE || role == TextureRole::SPECULAR
                       ? TextureContent::COLOR
                       : TextureContent::DATA;
        }

        bool is_greyscale(Yimage::PixelType type)
        {
            using enum Yimage::PixelType;
            switch (type)
            {
            case MONO_1:
            case MONO_2:
            case MONO_4:
            case MONO_8:
            case MONO_16:
            case MONO_FLOAT_32:
            case MONO_ALPHA_8:
            case MONO_ALPHA_16:
            case ALPHA_MONO_8:
            case ALPHA_MONO_16:
                return true;
            default:
                return false;
            }
        }

        Xyz::Vector3F to_vector(const tinyobj::real_t* values)
        {
            return {float(values[0]), float(values[1]), float(values[2])};
        }

        /**
         * MTL colors are taken as authored, which is what ColorMaterial
         * expects. See import_obj for why Ka is ignored.
         */
        ColorMaterial to_color_material(const tinyobj::material_t& material)
        {
            const auto diffuse = to_vector(material.diffuse);
            return {
                .ambient = linear_to_srgb(srgb_to_linear(diffuse)
                                          * AMBIENT_FRACTION),
                .diffuse = diffuse,
                .specular = to_vector(material.specular),
                .shininess = std::clamp(float(material.shininess),
                                        MIN_SHININESS, MAX_SHININESS)
            };
        }

        /**
         * A texture path from an MTL file, which may have been written on
         * Windows.
         */
        std::filesystem::path to_path(std::string name)
        {
            std::ranges::replace(name, '\\', '/');
            return std::filesystem::path(name);
        }

        /** One corner of an OBJ face: indices into the file's separate
         *  position, normal and texture coordinate arrays. */
        using CornerKey = std::tuple<int, int, int>;

        struct CornerKeyHash
        {
            size_t operator()(const CornerKey& key) const
            {
                const auto [v, n, t] = key;
                auto hash = std::hash<int>()(v);
                hash = hash * 31 + std::hash<int>()(n);
                return hash * 31 + std::hash<int>()(t);
            }
        };

        /**
         * Turns one OBJ file into Tungsten nodes, meshes, materials and
         * textures. One instance per import: it holds the caches that keep a
         * material or an image shared by several shapes from being built
         * more than once.
         */
        class ObjImporter
        {
        public:
            ObjImporter(const tinyobj::ObjReader& reader,
                        std::filesystem::path directory,
                        ResourceManager& resources,
                        Scene& scene,
                        NodeId parent,
                        const ObjImportOptions& options)
                : attrib_(reader.GetAttrib()),
                  shapes_(reader.GetShapes()),
                  materials_(reader.GetMaterials()),
                  directory_(std::move(directory)),
                  resources_(resources),
                  scene_(scene),
                  builder_(resources, scene, parent),
                  create_wireframes_(options.create_wireframes)
            {
                // The same format as the glTF importer's rigid meshes: the
                // Blinn-Phong family requires all three attributes.
                layout_ = resources_.register_layout(
                    VertexLayoutBuilder()
                        .add_attribute(AttributeSemantic::POSITION)
                        .add_attribute(AttributeSemantic::NORMAL)
                        .add_attribute(AttributeSemantic::TEX_COORD_0)
                        .build());
                shader_ = resources_.register_shader_variant(
                    {.family = BLINN_PHONG_FAMILY});
            }

            /**
             * Imports the file. If it throws, the builder removes whatever
             * had been added to the scene and the resource manager.
             */
            ObjImport run()
            {
                for (size_t i = 0; i < shapes_.size(); ++i)
                    import_shape(i);

                static_cast<Model&>(result_) = builder_.build();
                for (const auto& wireframe : pending_wireframes_)
                {
                    result_.wireframes.push_back({
                        .node = wireframe.node,
                        .mesh = builder_.get_mesh(wireframe.mesh),
                        .material = wireframe.material
                    });
                }
                return std::move(result_);
            }

            void warn(std::string message)
            {
                result_.warnings.push_back(std::move(message));
            }

        private:
            /**
             * Gives each material the shape uses a node with a renderable
             * of its own: a renderable is one mesh and one material.
             */
            void import_shape(size_t shape_index)
            {
                const auto& shape = shapes_[shape_index];
                const auto what = "shape '" + name_of(shape.name, shape_index)
                                  + "'";

                if (!shape.lines.indices.empty() || !shape.points.indices.empty())
                {
                    warn("skipped the lines and points of " + what
                         + ": they have no surface to shade.");
                }

                // The faces are triangles (the reader triangulates), listed
                // in material order so the output does not depend on a hash.
                const auto& mesh = shape.mesh;
                std::map<int, std::vector<size_t>> faces_by_material;
                size_t offset = 0;
                for (size_t face = 0; face < mesh.num_face_vertices.size(); ++face)
                {
                    const auto count = size_t(mesh.num_face_vertices[face]);
                    if (count == 3)
                    {
                        const auto material = face < mesh.material_ids.size()
                                                  ? mesh.material_ids[face]
                                                  : NO_MATERIAL;
                        faces_by_material[material].push_back(offset);
                    }
                    offset += count;
                }

                for (const auto& [material, faces] : faces_by_material)
                {
                    auto where = what;
                    if (faces_by_material.size() > 1)
                        where += " (material " + material_name(material) + ")";

                    auto data = read_faces(mesh, faces, where);
                    if (!data)
                        continue;

                    const auto id = scene_.create_node(builder_.root());
                    const auto vertices = builder_.add_vertices(
                        layout_, Detail::interleave(*data));
                    builder_.add_renderable(
                        id, builder_.add_mesh(vertices, data->indices),
                        get_material(material, TopologyType::TRIANGLES));
                    ++result_.primitive_count;

                    if (!create_wireframes_)
                        continue;

                    // Faces that are all degenerate have no edges, and a mesh
                    // cannot be empty.
                    auto line_indices = Detail::make_line_indices(
                        data->indices, TopologyType::TRIANGLES);
                    if (line_indices.empty())
                        continue;

                    pending_wireframes_.push_back({
                        .node = id,
                        .mesh = builder_.add_mesh(vertices, std::move(line_indices)),
                        .material = get_material(material, TopologyType::LINES)
                    });
                }
            }

            /**
             * The vertices and indices of the triangles starting at
             * @a corner_offsets, with each distinct combination of position,
             * normal and texture coordinate becoming one vertex.
             *
             * Normals are only used if every corner has one: complete_primitive
             * flat-shades a primitive that has none, and cannot complete one
             * that has some. Missing texture coordinates become zeroes.
             */
            std::optional<PrimitiveData>
            read_faces(const tinyobj::mesh_t& mesh,
                       const std::vector<size_t>& corner_offsets,
                       const std::string& where)
            {
                const auto position_count = attrib_.vertices.size() / 3;
                const auto normal_count = attrib_.normals.size() / 3;
                const auto tex_coord_count = attrib_.texcoords.size() / 2;

                bool has_normals = true;
                bool has_tex_coords = false;
                for (const auto offset : corner_offsets)
                {
                    for (size_t i = 0; i < 3; ++i)
                    {
                        const auto& index = mesh.indices[offset + i];
                        if (index.vertex_index < 0
                            || size_t(index.vertex_index) >= position_count
                            || size_t(index.normal_index + 1) > normal_count
                            || size_t(index.texcoord_index + 1) > tex_coord_count)
                        {
                            warn("skipped " + where + ": it refers to vertex"
                                 " data the file does not have.");
                            return {};
                        }
                        has_normals &= index.normal_index >= 0;
                        has_tex_coords |= index.texcoord_index >= 0;
                    }
                }

                PrimitiveData data;
                std::unordered_map<CornerKey, uint32_t, CornerKeyHash> vertices;
                data.indices.reserve(corner_offsets.size() * 3);
                for (const auto offset : corner_offsets)
                {
                    for (size_t i = 0; i < 3; ++i)
                    {
                        const auto& index = mesh.indices[offset + i];
                        const CornerKey key{
                            index.vertex_index,
                            has_normals ? index.normal_index : -1,
                            has_tex_coords ? index.texcoord_index : -1
                        };
                        const auto [it, inserted] = vertices.try_emplace(
                            key, uint32_t(data.positions.size()));
                        data.indices.push_back(it->second);
                        if (!inserted)
                            continue;

                        const auto [v, n, t] = key;
                        data.positions.push_back(
                            to_vector(&attrib_.vertices[3 * size_t(v)]));
                        if (has_normals)
                        {
                            data.normals.push_back(
                                to_vector(&attrib_.normals[3 * size_t(n)]));
                        }
                        if (has_tex_coords)
                        {
                            // OBJ puts v = 0 at the bottom of the image, glTF
                            // and Tungsten's image upload at the top.
                            data.tex_coords.push_back(
                                t < 0
                                    ? Xyz::Vector2F{0, 0}
                                    : Xyz::Vector2F{
                                        float(attrib_.texcoords[2 * size_t(t)]),
                                        1.0f - float(attrib_.texcoords[2 * size_t(t) + 1])
                                    });
                        }
                    }
                }

                Detail::complete_primitive(data);
                if (data.positions.empty() || data.indices.empty())
                {
                    warn("skipped " + where + ": it has no triangles.");
                    return {};
                }
                return data;
            }

            MaterialRef get_material(int index, TopologyType topology)
            {
                const auto key = std::pair(index, topology);
                if (const auto it = material_cache_.find(key);
                    it != material_cache_.end())
                {
                    return it->second;
                }

                Material material;
                if (index == NO_MATERIAL)
                {
                    material.pipeline = get_pipeline(topology, false);
                    material.parameter_data = make_blinn_phong_material_params(
                        ColorMaterial{});
                }
                else
                {
                    const auto& source = materials_[size_t(index)];
                    const auto opacity = std::clamp(float(source.dissolve),
                                                    0.0f, 1.0f);
                    material.pipeline = get_pipeline(topology, opacity < 1.0f);
                    material.textures = get_textures(source);
                    // A normal map that turned out to be a height map leaves
                    // the slot empty, and the strength must then be zero.
                    const auto normal_strength =
                        material.textures.size() > 2 && material.textures[2]
                            ? float(source.normal_texname.empty()
                                        ? source.bump_texopt.bump_multiplier
                                        : source.normal_texopt.bump_multiplier)
                            : 0.0f;
                    material.parameter_data = make_blinn_phong_material_params(
                        to_color_material(source), opacity, normal_strength);
                }

                const auto ref = builder_.add_material(std::move(material));
                material_cache_.emplace(key, ref);
                return ref;
            }

            /**
             * The material's textures in the slot order the Blinn-Phong family
             * fixes: diffuse, specular, normal. Trailing nulls are dropped.
             */
            std::vector<TextureRef> get_textures(const tinyobj::material_t& source)
            {
                std::vector<TextureRef> textures(3);
                if (!source.diffuse_texname.empty())
                {
                    textures[0] = get_texture(source.diffuse_texname,
                                              source.diffuse_texopt,
                                              TextureRole::DIFFUSE);
                }
                if (!source.specular_texname.empty())
                {
                    textures[1] = get_texture(source.specular_texname,
                                              source.specular_texopt,
                                              TextureRole::SPECULAR);
                }
                if (!source.normal_texname.empty())
                {
                    textures[2] = get_texture(source.normal_texname,
                                              source.normal_texopt,
                                              TextureRole::NORMAL_MAP);
                }
                else if (!source.bump_texname.empty())
                {
                    textures[2] = get_texture(source.bump_texname,
                                              source.bump_texopt,
                                              TextureRole::BUMP_MAP);
                }
                while (!textures.empty() && !textures.back())
                    textures.pop_back();
                return textures;
            }

            TextureRef get_texture(const std::string& name,
                                   const tinyobj::texture_option_t& options,
                                   TextureRole role)
            {
                const TextureKey key{name, role, options.clamp};
                if (const auto it = texture_cache_.find(key);
                    it != texture_cache_.end())
                {
                    return it->second;
                }

                TextureRef ref;
                try
                {
                    const auto image = read_image(
                        (directory_ / to_path(name)).string());
                    if (Yimage::get_pixel_size(image.pixel_type()) < 8)
                        TUNGSTEN_THROW("sub-byte pixels are not supported");

                    if (role == TextureRole::BUMP_MAP
                        && is_greyscale(image.pixel_type()))
                    {
                        warn("bump map '" + name + "' is a height map, which"
                             " is not supported; it is drawn without it.");
                    }
                    else
                    {
                        ref = builder_.add_texture({
                            .size = {int32_t(image.width()),
                                     int32_t(image.height())},
                            .format = get_ogl_pixel_type(image.pixel_type()),
                            .content = content_of(role),
                            .pixels = image.data(),
                            .sampler = get_sampler(options.clamp)
                        });
                    }
                }
                catch (const std::exception& e)
                {
                    // A missing or unreadable texture is a reason to draw the
                    // model without it, not to refuse the file.
                    warn("texture '" + name + "' could not be loaded ("
                         + e.what() + "); it is drawn without it.");
                }
                texture_cache_.emplace(key, ref);
                return ref;
            }

            SamplerRef get_sampler(bool clamp)
            {
                SamplerDescriptor descriptor;
                // No mipmaps are generated for imported textures, so asking
                // for a mip filter would sample a level that does not exist.
                descriptor.mip_filter = SamplerMipFilter::NONE;
                descriptor.min_filter = SamplerMinMagFilter::LINEAR;
                const auto mode = clamp ? SamplerAddressMode::CLAMP_TO_EDGE
                                        : SamplerAddressMode::REPEAT;
                descriptor.address_mode_u = mode;
                descriptor.address_mode_v = mode;
                return resources_.register_sampler(descriptor);
            }

            PipelineRef get_pipeline(TopologyType topology, bool transparent)
            {
                PipelineDescriptor descriptor;
                descriptor.shader = shader_;
                descriptor.layout = layout_;
                descriptor.primitive = topology;
                // OBJ says nothing about which side of a face is the front,
                // and files with inconsistent winding are common.
                descriptor.raster.cull_enabled = false;
                if (transparent)
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
                // Interned, so building a descriptor per material is free.
                return resources_.register_pipeline(descriptor);
            }

            std::string material_name(int index) const
            {
                if (index == NO_MATERIAL)
                    return "none";
                return name_of(materials_[size_t(index)].name, size_t(index));
            }

            static std::string name_of(std::string_view name, size_t index)
            {
                return name.empty() ? std::to_string(index) : std::string(name);
            }

            /** A wireframe whose mesh build() has yet to create. */
            struct PendingWireframe
            {
                NodeId node;
                ModelMeshId mesh = 0;
                MaterialRef material;
            };

            using MaterialKey = std::pair<int, TopologyType>;
            using TextureKey = std::tuple<std::string, TextureRole, bool>;

            const tinyobj::attrib_t& attrib_;
            const std::vector<tinyobj::shape_t>& shapes_;
            const std::vector<tinyobj::material_t>& materials_;
            std::filesystem::path directory_;
            ResourceManager& resources_;
            Scene& scene_;
            ModelBuilder builder_;
            ObjImport result_;
            VertexLayoutRef layout_;
            ShaderProgramRef shader_;
            std::map<MaterialKey, MaterialRef> material_cache_;
            std::map<TextureKey, TextureRef> texture_cache_;
            std::vector<PendingWireframe> pending_wireframes_;
            bool create_wireframes_ = false;
        };

        /**
         * tinyobjloader's warnings, one per line, without its trailing
         * newlines.
         */
        std::vector<std::string> split_lines(const std::string& text)
        {
            std::vector<std::string> result;
            size_t start = 0;
            while (start < text.size())
            {
                auto end = text.find('\n', start);
                if (end == std::string::npos)
                    end = text.size();
                if (end > start)
                    result.push_back(text.substr(start, end - start));
                start = end + 1;
            }
            return result;
        }
    }

    ObjImport import_obj(const std::filesystem::path& file_name,
                         ResourceManager& resources,
                         Scene& scene,
                         NodeId parent,
                         const ObjImportOptions& options)
    {
        if (!std::filesystem::is_regular_file(file_name))
            TUNGSTEN_THROW("" + file_name.string() + ": no such file.");

        tinyobj::ObjReaderConfig config;
        config.triangulate = true;
        config.vertex_color = false;

        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(file_name.string(), config))
        {
            TUNGSTEN_THROW("" + file_name.string() + ": " + reader.Error());
        }

        ObjImporter importer(reader, file_name.parent_path(), resources,
                             scene, parent, options);
        // Missing MTL files and the like are reported here, before the
        // problems the importer finds.
        for (auto& line : split_lines(reader.Warning()))
            importer.warn(std::move(line));
        auto result = importer.run();

        if (result.primitive_count == 0)
        {
            remove_model(result, scene, resources);
            TUNGSTEN_THROW("" + file_name.string() + ": the file contains nothing"
                           " that can be displayed. Only faces are supported."
                           + describe(result.warnings));
        }
        if (!result.bounds)
        {
            remove_model(result, scene, resources);
            TUNGSTEN_THROW("" + file_name.string() + ": the file's geometry has no"
                           " extent, so there is nothing to look at."
                           + describe(result.warnings));
        }

        return result;
    }
} // Tungsten
