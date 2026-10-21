//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Animation/AnimationClip.hpp"

#include <algorithm>
#include <cmath>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        // Above this dot product the arc is too short for slerp's division by
        // its sine to be accurate, and normalized lerp is indistinguishable.
        constexpr float SLERP_THRESHOLD = 0.9995f;

        float dot(const Xyz::QuaternionF& a, const Xyz::QuaternionF& b)
        {
            return a.w * b.w + Xyz::dot(a.v, b.v);
        }

        /**
         * The cubic Hermite spline from p0 to p1. m0 and m1 are the tangents
         * already scaled to the segment's length, which is what glTF's
         * per-second tangents need before they can be used.
         */
        template <typename T>
        T hermite(const T& p0, const T& m0, const T& p1, const T& m1, float u)
        {
            const auto u2 = u * u;
            const auto u3 = u2 * u;
            return p0 * (2 * u3 - 3 * u2 + 1)
                   + m0 * (u3 - 2 * u2 + u)
                   + p1 * (3 * u2 - 2 * u3)
                   + m1 * (u3 - u2);
        }

        template <typename T, typename Lerp>
        T sample_track(const AnimationTrack<T>& track, float time, Lerp lerp)
        {
            const auto& times = track.times;
            const auto& values = track.values;

            // Written as a negation so that NaN also lands here, rather than
            // making upper_bound return the end.
            if (!(time > times.front()))
                return values.front();
            if (time >= times.back())
                return values.back();

            const auto it = std::upper_bound(times.begin(), times.end(), time);
            const auto next = size_t(it - times.begin());
            const auto prev = next - 1;
            const auto span = times[next] - times[prev];
            const auto u = (time - times[prev]) / span;

            switch (track.interpolation)
            {
            case Interpolation::STEP:
                return values[prev];
            case Interpolation::LINEAR:
                return lerp(values[prev], values[next], u);
            case Interpolation::CUBIC_SPLINE:
                return hermite(values[prev], track.out_tangents[prev] * span,
                               values[next], track.in_tangents[next] * span,
                               u);
            }
            return values[prev];
        }

        template <typename T>
        void validate_track(const AnimationTrack<T>& track,
                            const std::string& where)
        {
            const auto& times = track.times;
            if (times.empty())
                TUNGSTEN_THROW("" + where + " has no keyframes.");
            if (track.values.size() != times.size())
            {
                TUNGSTEN_THROW("" + where + " has a different number of values"
                               " and times.");
            }

            for (size_t i = 0; i < times.size(); ++i)
            {
                if (!std::isfinite(times[i]))
                {
                    TUNGSTEN_THROW("" + where + " has a time that is not"
                                   " finite.");
                }
                if (i != 0 && times[i] <= times[i - 1])
                {
                    TUNGSTEN_THROW("" + where + "'s times are not strictly"
                                   " increasing.");
                }
            }

            const auto tangent_count
                = track.interpolation == Interpolation::CUBIC_SPLINE
                      ? times.size()
                      : 0;
            if (track.in_tangents.size() != tangent_count
                || track.out_tangents.size() != tangent_count)
            {
                TUNGSTEN_THROW("" + where + " has tangents that do not match"
                               " its interpolation.");
            }
        }
    }

    Xyz::Vector3F sample(const Vector3Track& track, float time)
    {
        const auto lerp = [](const auto& a, const auto& b, float u)
        {
            return a + (b - a) * u;
        };
        return sample_track(track, time, lerp);
    }

    Xyz::QuaternionF sample(const RotationTrack& track, float time)
    {
        const auto result = sample_track(track, time, slerp);
        // The spline does not stay on the unit sphere between keyframes.
        if (track.interpolation == Interpolation::CUBIC_SPLINE)
            return Xyz::normalize(result);
        return result;
    }

    Xyz::QuaternionF slerp(const Xyz::QuaternionF& a,
                           const Xyz::QuaternionF& b,
                           float t)
    {
        // q and -q are the same rotation; flipping b when the two point
        // apart takes the short way round.
        auto d = dot(a, b);
        auto end = b;
        if (d < 0)
        {
            end = -b;
            d = -d;
        }

        if (d > SLERP_THRESHOLD)
            return Xyz::normalize(a + (end - a) * t);

        const auto angle = std::acos(d);
        const auto sine = std::sin(angle);
        return a * (std::sin((1 - t) * angle) / sine)
               + end * (std::sin(t * angle) / sine);
    }

    AnimationClip make_animation_clip(std::string name,
                                      std::vector<AnimationChannel> channels)
    {
        float duration = 0;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            const auto& channel = channels[i];
            const auto where = "channel " + std::to_string(i)
                               + " of animation '" + name + "'";

            const auto is_rotation = channel.path == AnimationPath::ROTATION;
            const auto has_rotation
                = std::holds_alternative<RotationTrack>(channel.track);
            if (is_rotation != has_rotation)
            {
                TUNGSTEN_THROW("" + where + " has a track of the wrong type for"
                               " its path.");
            }

            std::visit([&](const auto& track)
            {
                validate_track(track, where);
                duration = std::max(duration, track.times.back());
            }, channel.track);
        }

        return {
            .name = std::move(name),
            .channels = std::move(channels),
            .duration = duration
        };
    }
} // Tungsten
