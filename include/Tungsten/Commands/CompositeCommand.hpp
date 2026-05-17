//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-15.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Command.hpp"
#include <memory>
#include <vector>

namespace Tungsten
{
    class CompositeCommand : public Command
    {
    public:
        CompositeCommand() = default;

        explicit CompositeCommand(std::vector<std::shared_ptr<Command>> commands)
            : _commands(std::move(commands))
        {
        }

        void add_command(std::shared_ptr<Command> command)
        {
            _commands.push_back(std::move(command));
        }

        void run(const Camera& camera) override
        {
            for (const auto& command : _commands)
                command->run(camera);
        }
    private:
        std::vector<std::shared_ptr<Command>> _commands;
    };
} // Tungsten
