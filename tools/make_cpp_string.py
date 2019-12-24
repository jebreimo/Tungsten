#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
# ===========================================================================
# Copyright © 2019 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2019-04-26.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
# ===========================================================================
import os
import sys


"""Creates a c++ multi-line string containing the contents of the given file. 
"""


def make_variable_name(file_name):
    source_name = os.path.splitext(os.path.basename(file_name))[0]
    if not source_name:
        return "text"
    variable_name = []
    if source_name[0].isdigit():
        variable_name.append("_")
    else:
        variable_name.append("")
    for i, c in enumerate(source_name):
        if c.isalnum() or c == "_":
            variable_name.append(c)
        elif variable_name[-1] != "_":
            variable_name.append("_")
    return "".join(variable_name)


def main(args):
    if len(args) not in (1, 2):
        print("usage: %s <shader file> [output file]"
              % os.path.basename(sys.argv[0]))
        return 1
    input = open(args[0])
    if len(args) == 2:
        dir_name = os.path.dirname(args[1])
        if dir_name and not os.path.exists(dir_name):
            os.makedirs(dir_name)
        output = open(args[1], "w")
    else:
        output = sys.stdout
    output.write('constexpr char %s[] = R"---(' % make_variable_name(args[0]))
    output.write(input.read())
    output.write(')---";\n')
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
