//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "../SceneGraph/NodeId.hpp"
#include "../SceneGraph/Transform.hpp"
#include "AnimationClip.hpp"

namespace Tungsten
{
    class Scene;

    /**
     * The nodes that AnimationChannel::target indices refer to, and each
     * node's transform when no animation touches it.
     *
     * Both vectors are indexed by target. A null node means the target has
     * no node in this instance; channels aimed at it are ignored.
     */
    struct AnimationTargets
    {
        std::vector<NodeId> nodes;
        std::vector<Transform> rest_transforms;
    };

    enum class PlaybackMode : uint8_t
    {
        /** Stops at the end of the clip, or at its start if playing
         *  backwards. */
        ONCE,
        /** Wraps around to the other end of the clip. */
        LOOP
    };

    /**
     * Plays one clip on one set of nodes.
     *
     * Every apply starts each animated node from its rest transform and
     * overwrites the parts the clip animates, so a channel that only rotates
     * a node leaves its translation and scale as they were imported. The
     * node's local transform is replaced outright: two players animating the
     * same node do not blend, and whichever applies last wins.
     *
     * Nodes that have been removed from the scene are skipped, so a player
     * outliving its nodes is harmless.
     */
    class AnimationPlayer
    {
    public:
        AnimationPlayer() = default;

        /**
         * @throw TungstenException if @a clip is null, if a channel's target
         *      is outside @a targets, or if the two vectors in @a targets
         *      differ in size.
         */
        AnimationPlayer(std::shared_ptr<const AnimationClip> clip,
                        const AnimationTargets& targets,
                        PlaybackMode mode = PlaybackMode::LOOP);

        /** The clip, or null for a default-constructed player. */
        [[nodiscard]]
        const std::shared_ptr<const AnimationClip>& clip() const;

        /** The position in the clip, in seconds. */
        [[nodiscard]]
        float time() const;

        /**
         * Moves to @a time, wrapped or clamped to the clip according to the
         * playback mode.
         */
        void set_time(float time);

        /**
         * How many clip seconds advance moves per real second. Negative
         * plays the clip backwards.
         */
        [[nodiscard]]
        float speed() const;

        void set_speed(float speed);

        [[nodiscard]]
        PlaybackMode mode() const;

        void set_mode(PlaybackMode mode);

        /**
         * Returns true if the mode is ONCE and time has reached the end it
         * is moving towards.
         */
        [[nodiscard]]
        bool is_finished() const;

        /** Moves the time forward by @a seconds times the speed. */
        void advance(float seconds);

        /**
         * Sets the local transform of every node the clip animates to its
         * pose at the current time.
         */
        void apply(Scene& scene);

        /** advance followed by apply. */
        void update(Scene& scene, float seconds);

        /**
         * Puts every node the clip animates back in its rest transform. Call
         * it before switching a model to a clip that animates fewer nodes,
         * or the ones left out stay where this clip put them.
         */
        void restore_rest_pose(Scene& scene) const;

    private:
        std::shared_ptr<const AnimationClip> clip_;
        /** The distinct nodes the clip animates, and their rest transforms. */
        std::vector<NodeId> nodes_;
        std::vector<Transform> rest_transforms_;
        /** Per channel, its index in nodes_, or NO_SLOT if it is ignored. */
        std::vector<uint32_t> channel_slots_;
        /** Scratch space for apply, kept to avoid allocating every frame. */
        std::vector<Transform> pose_;
        float time_ = 0;
        float speed_ = 1;
        PlaybackMode mode_ = PlaybackMode::LOOP;
    };
} // Tungsten
