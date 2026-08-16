//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

namespace Tungsten
{
    class Scene;

    // Finalizes the scene's world transforms once per frame, before the
    // SnapshotBuilder extracts them (§2, §5).
    //
    // The work itself lives in Scene::resolve_transforms, because it is a loop
    // over the scene's own arrays; this class remains as the named step in the
    // frame pipeline that callers already write.
    class TransformUpdater
    {
    public:
        static void resolve(Scene& scene);
    };
} // Tungsten