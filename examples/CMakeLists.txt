# ===========================================================================
# Copyright © 2025 Jan Erik Breimo. All rights reserved.
# Created by Jan Erik Breimo on 2025-12-03.
#
# This file is distributed under the Zero-Clause BSD License.
# License text is included with the source distribution.
# ===========================================================================
add_subdirectory(bump_cube)
add_subdirectory(cube)
# The glTF importer is not part of the Emscripten build, so neither is its
# viewer.
if (NOT EMSCRIPTEN)
    add_subdirectory(gltf_viewer)
endif ()
add_subdirectory(neo_table)
add_subdirectory(scene_graph)
add_subdirectory(show_text)
add_subdirectory(fading_blob)
add_subdirectory(touch_events)
