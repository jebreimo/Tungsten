//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SceneGraph/ChildIterator.hpp"

#include "Tungsten/SceneGraph/NodeHandle.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"

namespace Tungsten
{
    NodeHandle ChildIterator::operator*() const
    {
        return {scene_, id_};
    }

    ChildIterator& ChildIterator::operator++()
    {
        id_ = scene_->next_sibling(id_);
        return *this;
    }
}
