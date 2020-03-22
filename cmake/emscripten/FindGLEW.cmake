#****************************************************************************
# Copyright © 2020 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2020-03-22.
#
# This file is distributed under the BSD License.
# License text is included with the source distribution.
#****************************************************************************
if (NOT TARGET GLEW::GLEW)
    add_library(GLEW::GLEW INTERFACE IMPORTED)
endif ()

set(GLEW_FOUND YES)
