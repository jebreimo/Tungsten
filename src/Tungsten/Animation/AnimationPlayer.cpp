//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Animation/AnimationPlayer.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"

namespace Tungsten
{
    namespace
    {
        constexpr uint32_t NO_SLOT = UINT32_MAX;
    }

    AnimationPlayer::AnimationPlayer(std::shared_ptr<const AnimationClip> clip,
                                     const AnimationTargets& targets,
                                     PlaybackMode mode)
        : clip_(std::move(clip)),
          mode_(mode)
    {
        if (!clip_)
            TUNGSTEN_THROW("AnimationPlayer: the clip is null.");
        if (targets.nodes.size() != targets.rest_transforms.size())
        {
            TUNGSTEN_THROW("AnimationPlayer: the targets have a different"
                           " number of nodes and rest transforms.");
        }

        // Several channels usually animate the same node, one per path; they
        // share a slot so that each node is written once per apply.
        std::unordered_map<uint32_t, uint32_t> slots;
        channel_slots_.reserve(clip_->channels.size());
        for (const auto& channel : clip_->channels)
        {
            if (channel.target >= targets.nodes.size())
            {
                TUNGSTEN_THROW("AnimationPlayer: animation '" + clip_->name
                               + "' targets a node outside the targets.");
            }

            const auto node = targets.nodes[channel.target];
            if (!node)
            {
                channel_slots_.push_back(NO_SLOT);
                continue;
            }

            const auto [it, added] = slots.try_emplace(
                channel.target, uint32_t(nodes_.size()));
            if (added)
            {
                nodes_.push_back(node);
                rest_transforms_.push_back(
                    targets.rest_transforms[channel.target]);
            }
            channel_slots_.push_back(it->second);
        }
    }

    const std::shared_ptr<const AnimationClip>& AnimationPlayer::clip() const
    {
        return clip_;
    }

    float AnimationPlayer::time() const
    {
        return time_;
    }

    void AnimationPlayer::set_time(float time)
    {
        const auto duration = clip_ ? clip_->duration : 0.0f;
        if (!(duration > 0) || !std::isfinite(time))
        {
            time_ = 0;
            return;
        }

        if (mode_ == PlaybackMode::LOOP)
        {
            time_ = std::fmod(time, duration);
            if (time_ < 0)
                time_ += duration;
        }
        else
        {
            time_ = std::clamp(time, 0.0f, duration);
        }
    }

    float AnimationPlayer::speed() const
    {
        return speed_;
    }

    void AnimationPlayer::set_speed(float speed)
    {
        speed_ = speed;
    }

    PlaybackMode AnimationPlayer::mode() const
    {
        return mode_;
    }

    void AnimationPlayer::set_mode(PlaybackMode mode)
    {
        mode_ = mode;
    }

    bool AnimationPlayer::is_finished() const
    {
        if (mode_ != PlaybackMode::ONCE || !clip_)
            return false;
        return speed_ >= 0 ? time_ >= clip_->duration : time_ <= 0;
    }

    void AnimationPlayer::advance(float seconds)
    {
        set_time(time_ + seconds * speed_);
    }

    void AnimationPlayer::apply(Scene& scene)
    {
        if (!clip_)
            return;

        pose_.assign(rest_transforms_.begin(), rest_transforms_.end());
        for (size_t i = 0; i < clip_->channels.size(); ++i)
        {
            const auto slot = channel_slots_[i];
            if (slot == NO_SLOT)
                continue;

            const auto& channel = clip_->channels[i];
            auto& transform = pose_[slot];
            switch (channel.path)
            {
            case AnimationPath::TRANSLATION:
                transform.translation = sample(
                    std::get<Vector3Track>(channel.track), time_);
                break;
            case AnimationPath::ROTATION:
                transform.rotation = sample(
                    std::get<RotationTrack>(channel.track), time_);
                break;
            case AnimationPath::SCALE:
                transform.scale = sample(
                    std::get<Vector3Track>(channel.track), time_);
                break;
            }
        }

        for (size_t i = 0; i < nodes_.size(); ++i)
        {
            if (scene.is_alive(nodes_[i]))
                scene.set_local_transform(nodes_[i], pose_[i]);
        }
    }

    void AnimationPlayer::update(Scene& scene, float seconds)
    {
        advance(seconds);
        apply(scene);
    }

    void AnimationPlayer::restore_rest_pose(Scene& scene) const
    {
        for (size_t i = 0; i < nodes_.size(); ++i)
        {
            if (scene.is_alive(nodes_[i]))
                scene.set_local_transform(nodes_[i], rest_transforms_[i]);
        }
    }
} // Tungsten
