//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <ostream>

namespace argos
{
    class ArgumentParser;

    class ParsedArguments;
}

namespace Tungsten
{
    class SdlApplication;

    void add_command_line_options(argos::ArgumentParser& parser);

    void read_command_line_options(const argos::ParsedArguments& args,
                                   SdlApplication& app);

    void parse_command_line_options(int& argc, char**& argv,
                                    SdlApplication& app);
}
