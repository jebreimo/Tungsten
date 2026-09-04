# Overlay triplet: the stock arm64-osx settings, plus a rule that tells every
# port's CMake configure to ignore XQuartz's /opt/X11 tree (and its /usr/X11R6
# symlink). Without this, ports such as glew resolve their OpenGL dependency to
# /opt/X11/lib/libGL and bake that absolute path into their exported CMake
# targets, which then collides with the system OpenGL.framework that SDL uses
# (see the OpenGL/GLEW notes in CMakeLists.txt). Selected via CMakePresets.json.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

# The escaped semicolon keeps this a single -D whose value is the CMake list
# "/opt/X11;/usr/X11R6".
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
    "-DCMAKE_IGNORE_PREFIX_PATH=/opt/X11\;/usr/X11R6")