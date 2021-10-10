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

    void addCommandLineOptions(argos::ArgumentParser& parser);

    void readCommandLineOptions(const argos::ParsedArguments& args,
                                SdlApplication& app);

    void parseCommandLineOptions(int& argc, char**& argv,
                                 SdlApplication& app);
}
