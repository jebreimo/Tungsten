//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// A minimal glTF viewer: load the file named on the command line, centre it,
// and turn it slowly under a two-light studio rig.
//
// The model is framed by its bounding *sphere* rather than its box. A sphere
// is what stays put while the model rotates, so the framing computed once at
// startup is still correct a quarter turn later; framing the box would make
// the model breathe in and out as its silhouette changed.
//
// Everything draws through the builtin Blinn-Phong family — see
// Tungsten/Import/GltfImport.hpp for how the file's metallic-roughness
// materials are approximated to reach it.
//
// The controls are drawn with Dear ImGui, on top of the scene: a button that
// switches between drawing the model's surfaces and the edges of its
// triangles, and, for a file with animations, a list to pick the one that
// plays. The first animation starts playing as soon as the file is loaded.
//
// Dropping another glTF or GLB file on the window replaces the model. This is
// the usage reference for unloading: the old model's nodes are removed from
// the scene, and the ResourceBundle its import returned is destroyed.

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <Argos/Argos.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <Tungsten/Tungsten.hpp>
#include <Tungsten/Gl/GlStateEpoch.hpp>
#include <Tungsten/Import/GltfImport.hpp>

namespace
{
    using namespace Tungsten;

    // Radians per second about +y. Slow enough to study the model, fast
    // enough that it is obviously moving.
    constexpr float ROTATION_SPEED = 0.3f;

    // How much wider than the model the frame is. A little air stops the
    // silhouette touching the window edge as it turns.
    constexpr float FRAMING_MARGIN = 1.15f;

    // The distance from the controls to the window's edges.
    constexpr float UI_MARGIN = 10.0f;

    // Wide enough for a typical animation name.
    constexpr float ANIMATION_LIST_WIDTH = 160.0f;

    // The animation list's entry for playing none of them.
    constexpr int NO_ANIMATION = -1;

    /**
     * Owns the ImGui context and its SDL and OpenGL backends. Needs a current
     * GL context for as long as it lives.
     */
    class ImGuiSession
    {
    public:
        ImGuiSession(SDL_Window* window, SDL_GLContext gl_context)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            // The viewer has no layout worth remembering between runs.
            ImGui::GetIO().IniFilename = nullptr;
            ImGui::StyleColorsDark();
            ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
            // The oldest context Tungsten asks for on the desktop is 3.3 core.
            ImGui_ImplOpenGL3_Init("#version 330 core");
        }

        ~ImGuiSession()
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
        }

        ImGuiSession(const ImGuiSession&) = delete;
        ImGuiSession& operator=(const ImGuiSession&) = delete;

        void begin_frame()
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }

        void end_frame()
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            // ImGui binds behind the back of Tungsten's state caches.
            notify_gl_state_changed();
        }
    };

    class GltfViewer : public EventLoop
    {
    public:
        GltfViewer(SdlApplication& app, const std::filesystem::path& file_name)
            : EventLoop(app),
              imgui_(app.window(), app.gl_context()),
              grid_(resources_),
              renderer_(resources_)
        {
            register_builtin_shader_families(resources_);

            grid_.add_to(scene_);

            pivot_ = scene_.add_node();
            camera_ = scene_.add_node();
            camera_.add(CameraComponent{.aspect = app.viewport().aspect_ratio()});
            load_model(file_name);

            // A key and a dimmer fill from the opposite side, so that the
            // model stays readable through a full turn. Their intensities sum
            // to less than one: a glTF base color is an albedo, and lighting
            // it with more than it reflects blows the highlights out.
            add_light({-0.5f, 0.6f, 0.0f}, {1.0f, 0.97f, 0.9f}, 0.75f);
            add_light({0.3f, -2.4f, 0.0f}, {0.8f, 0.85f, 1.0f}, 0.25f);

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        bool on_event(const SDL_Event& event) override
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_DROP_FILE)
            {
                // A file that cannot be shown is reported, not fatal: the
                // current model stays.
                try
                {
                    load_model(event.drop.data);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error: " << e.what() << '\n';
                }
                return true;
            }

            // Claim only what the controls are using, so that Escape still
            // quits and a click beside the button is still the scene's.
            const auto& io = ImGui::GetIO();
            switch (event.type)
            {
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_WHEEL:
                return io.WantCaptureMouse;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_TEXT_INPUT:
                return io.WantCaptureKeyboard;
            default:
                return false;
            }
        }

        void on_update() override
        {
            const auto ticks = SDL_GetTicks();
            const auto t = float(ticks - start_ticks_) / 1000.0f;
            pivot_.set_local_transform(
                Transform{.rotation = euler_rotation(0, ROTATION_SPEED * t, 0)});

            // Before on_draw resolves the transforms, so the pose shows in
            // the same frame.
            player_.update(scene_, float(ticks - update_ticks_) / 1000.0f);
            update_ticks_ = ticks;
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.12f, 0.13f, 0.16f, 1.0f}}
            };

            // Through the handle, not a cached pointer: the component arrays
            // move their contents as they grow.
            auto& camera = camera_.get<CameraComponent>();
            if (camera.aspect != viewport.aspect_ratio())
            {
                camera.aspect = viewport.aspect_ratio();
                place_camera();
            }

            // Before the snapshot is built, so that a click on the button
            // shows in this frame.
            imgui_.begin_frame();
            draw_controls();
            draw_frame_rate();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots_.back().ambient_light =
                srgb_to_linear(Xyz::Vector3F{0.20f, 0.21f, 0.25f});
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);
            imgui_.end_frame();

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        /**
         * Shows the model in @a file_name in place of the current one, and
         * reframes the camera for it. The new file is imported before the
         * old model is unloaded, so a file that fails to import leaves the
         * current model on screen.
         */
        void load_model(const std::filesystem::path& file_name)
        {
            auto import = import_gltf(file_name, resources_, scene_,
                                      pivot_.id(), {.create_wireframes = true});
            for (const auto& warning : import.warnings)
                std::cerr << "Warning: " << warning << '\n';

            unload_model();

            // The pivot turns; the model hangs under it shifted so that its
            // centre sits on the pivot. That is what makes it spin in place
            // rather than orbit.
            const auto centre = (import.bounds.min + import.bounds.max) / 2.0f;
            radius_ = Xyz::get_length(import.bounds.max - import.bounds.min) / 2;
            NodeHandle(&scene_, import.root).set_local_transform(
                Transform{.translation = -centre});

            alternates_ = std::move(import.wireframes);
            model_ = std::move(import);
            play_animation(model_.animations.empty() ? NO_ANIMATION : 0);

            // An import draws triangles; keep showing lines if that was the
            // mode the previous model was in.
            if (std::exchange(wireframe_, false))
                toggle_wireframe();

            place_camera();
        }

        /**
         * Removes the current model's nodes and everything its import
         * created.
         */
        void unload_model()
        {
            if (!model_.root)
                return;

            player_ = {};
            remove_model(model_, scene_, resources_);
            alternates_.clear();
        }

        /**
         * Backs the camera off far enough that a sphere of radius_ at the
         * origin fits the viewport, and sets the depth range to straddle it.
         */
        void place_camera()
        {
            auto& camera = camera_.get<CameraComponent>();

            // The vertical field of view is the given one; the horizontal
            // follows from the aspect ratio, and is the tighter of the two on
            // a window taller than it is wide.
            const auto half_v = camera.fov / 2;
            const auto half_h = std::atan(std::tan(half_v) * camera.aspect);
            const auto half_angle = std::min(half_v, half_h);

            const auto distance = FRAMING_MARGIN * radius_ / std::sin(half_angle);
            camera.near_plane = std::max(distance - radius_, distance * 1e-3f);
            camera.far_plane = distance + 2 * radius_;
            constexpr auto pitch = 0.4f;
            const auto position = distance * Xyz::Vector3F{0, sin(pitch), cos(pitch)};
            camera_.set_local_transform(
                Transform{
                    .translation = position,
                    .rotation = look_at_rotation(position, {0, 0, 0})
            });
        }

        /**
         * Builds the controls in the window's upper right corner.
         */
        void draw_controls()
        {
            const auto& display = ImGui::GetIO().DisplaySize;
            ImGui::SetNextWindowPos({display.x - UI_MARGIN, UI_MARGIN},
                                    ImGuiCond_Always, {1.0f, 0.0f});
            ImGui::Begin("Controls", nullptr,
                         ImGuiWindowFlags_NoDecoration
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_AlwaysAutoResize
                         | ImGuiWindowFlags_NoBackground);
            // The label names the mode the button switches to; the part after
            // ### is the id, which must not change with it.
            if (ImGui::Button(wireframe_ ? "Triangles###mode" : "Lines###mode"))
                toggle_wireframe();
            if (!model_.animations.empty())
                draw_animation_list();
            ImGui::End();
        }

        /**
         * Builds the list that picks which of the model's animations plays.
         */
        void draw_animation_list()
        {
            const auto name_of = [&](int index)
            {
                return index == NO_ANIMATION
                           ? "No animation"
                           : model_.animations[size_t(index)]->name.c_str();
            };

            ImGui::SetNextItemWidth(ANIMATION_LIST_WIDTH);
            if (!ImGui::BeginCombo("##animation", name_of(animation_index_)))
                return;

            for (int i = NO_ANIMATION; i < int(model_.animations.size()); ++i)
            {
                // The name need not be unique, so the index is the id.
                ImGui::PushID(i);
                const auto selected = i == animation_index_;
                if (ImGui::Selectable(name_of(i), selected) && !selected)
                    play_animation(i);
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        /**
         * Starts the model's animation @a index from the beginning, or stops animating if
         * index is NO_ANIMATION. The previous animation's nodes are put back
         * in their rest pose first: the next one need not animate them all.
         */
        void play_animation(int index)
        {
            player_.restore_rest_pose(scene_);
            if (index == NO_ANIMATION)
                player_ = {};
            else
                player_ = {model_.animations[size_t(index)], model_.targets};
            animation_index_ = index;
        }

        /**
         * Shows the frame rate in the window's lower right corner.
         */
        void draw_frame_rate()
        {
            const auto& io = ImGui::GetIO();
            ImGui::SetNextWindowPos({io.DisplaySize.x - UI_MARGIN,
                                     io.DisplaySize.y - UI_MARGIN},
                                    ImGuiCond_Always, {1.0f, 1.0f});
            // NoInputs: it is only a readout, so clicks on it go to the scene.
            ImGui::Begin("Frame rate", nullptr,
                         ImGuiWindowFlags_NoDecoration
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_AlwaysAutoResize
                         | ImGuiWindowFlags_NoBackground
                         | ImGuiWindowFlags_NoInputs);
            // ImGui's own figure, an average over its last 120 frames.
            ImGui::Text("%.0f FPS", io.Framerate);
            ImGui::End();
        }

        /**
         * Switches every imported renderable between its triangle mesh and
         * its line mesh.
         */
        void toggle_wireframe()
        {
            for (auto& alternate : alternates_)
            {
                auto& renderable =
                    scene_.get_component<RenderableComponent>(alternate.node);
                std::swap(renderable.mesh, alternate.mesh);
                std::swap(renderable.material, alternate.material);
            }
            wireframe_ = !wireframe_;
        }

        /**
         * Adds a directional light aimed by rotating its node: a light shines
         * along its node's -z axis.
         */
        void add_light(const Xyz::Vector3F& euler,
                       const Xyz::Vector3F& color, float intensity)
        {
            auto node = scene_.add_node();
            node.set_local_transform(Transform{
                .rotation = euler_rotation(euler[0], euler[1], euler[2])
            });
            node.add(LightComponent{
                .type = LightType::DIRECTIONAL,
                .color = color,
                .intensity = intensity
            });
        }

        ImGuiSession imgui_;
        ResourceManager resources_;
        InfiniteGrid grid_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        NodeHandle pivot_;
        NodeHandle camera_;
        // The current model: its nodes, its resources and its animations.
        Model model_;
        // The mesh and material each imported renderable is not currently
        // drawn with: the line ones to begin with, and after every toggle
        // whichever pair was swapped out.
        std::vector<GltfWireframe> alternates_;
        // The current model's animation that is playing.
        AnimationPlayer player_;
        int animation_index_ = NO_ANIMATION;
        bool wireframe_ = false;
        float radius_ = 1.0f;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
        uint64_t update_ticks_ = start_ticks_;
    };
}

int main(int argc, char** argv)
{
    try
    {
        argos::ArgumentParser parser(argv[0]);
        parser.about("Displays a glTF model, centred and slowly rotating.")
            .add(argos::Argument("FILE")
                .help("The glTF or GLB file to display."));
        Tungsten::SdlApplication::add_command_line_options(parser);
        const auto args = parser.parse(argc, argv);

        Tungsten::SdlApplication app("GltfViewer");
        app.read_command_line_options(args);
        app.run<GltfViewer>(args.value("FILE").as_string());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
