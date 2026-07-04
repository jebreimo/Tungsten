//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlProgram.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Gl/GlVertexArray.hpp"

namespace Tungsten
{
    // The frame-tagged deferred-deletion machinery (§11), owned by
    // ResourceManager and shared by every subsystem that takes a GL object out
    // of service: an arena's displaced BufferHandle after a grow, a destroyed
    // resource's ProgramHandle / TextureHandle, and a VertexArrayHandle evicted
    // from the VaoCache.
    //
    // A GL object is not deleted the moment it is retired, because an in-flight
    // frame (single-threaded: the frame being drawn; threaded: a snapshot behind
    // a fence) may still reference it. Instead its owning RAII handle is moved
    // onto the queue tagged with the current frame, and freed only once a
    // completed frame proves nothing references it — dropping the handle there is
    // what runs the GL delete.
    //
    // The handle types are distinct GlHandle specializations with no common base,
    // so the queue stores an owning variant over the four kinds. The variant is
    // move-only, exactly like the handles it holds.
    class DeletionQueue
    {
    public:
        using RetiredHandle = std::variant<BufferHandle, ProgramHandle,
                                           TextureHandle, VertexArrayHandle>;

        // Records the id of the frame about to be built / submitted. Everything
        // retired until the next call is tagged with it.
        void begin_frame(uint64_t frame);

        // Takes ownership of a GL handle and defers its deletion until a frame
        // at least as recent as the current one has completed. An empty handle
        // is harmless (its deleter is a no-op).
        void retire(RetiredHandle handle);

        // Deletes every handle retired in a frame the GPU has finished with,
        // i.e. tagged `<= completed_frame`. Single-threaded, completed_frame is
        // the just-drawn frame; with a render thread it is the latest passed
        // fence.
        void collect_garbage(uint64_t completed_frame);

    private:
        struct RetiredObject
        {
            RetiredHandle handle;
            uint64_t frame; // frame in which it was retired
        };

        std::vector<RetiredObject> entries_;
        uint64_t current_frame_ = 0;
    };
} // Tungsten