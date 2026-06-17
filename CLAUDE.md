# CLAUDE.md

Guidance for working in the Tungsten repository.

## What this is

Tungsten is a static C++20 library for building OpenGL applications on top of
SDL3. It targets both desktop (GLEW + OpenGL + SDL3 + Freetype) and the web
(Emscripten / WebGL). It provides thin, exception-safe wrappers over OpenGL
objects, a shader/material/lighting layer, text rendering, mesh utilities, and
an SDL application scaffold.

It is one of several `jebreimo` libraries and depends on sibling projects
(Xyz for math, Yimage, Yconvert, Argos, cppembed), pulled in via CMake
`FetchContent`.

## Layout

- `include/Tungsten/` — public headers. `Tungsten.hpp` is the umbrella include.
  Subdirs: `Gl/` (OpenGL object wrappers), `ShaderPrograms/`, `Commands/`,
  `Detail/` (internal helpers).
- `src/Tungsten/` — implementation. Mirrors the include tree, plus
  `Shaders/` (GLSL sources) and `TextRenderer/`.
- `examples/` — runnable sample apps (`cube`, `fading_blob`, `show_text`,
  `touch_events`). Treat these as the usage reference and keep them building.
- `tests/TungstenTest/` — Catch2 v3 unit tests.
- `cmake-build-*/` — out-of-source build dirs (debug, release, emscripten).
  Generated; never edit by hand.

## Build & test

Configure/build with an existing build directory, e.g.:

```sh
cmake --build cmake-build-debug                  # whole project
cmake --build cmake-build-debug --target Tungsten # library only
ctest --test-dir cmake-build-debug                # run tests
```

- C++ standard is **C++20**; CMake minimum is 3.15.
- Tests use **Catch2 v3**. Add new test files to
  `tests/TungstenTest/CMakeLists.txt`.
- Options: `TUNGSTEN_BUILD_TEST`, `TUNGSTEN_BUILD_EXAMPLES`, `TUNGSTEN_INSTALL`
  (all default ON when Tungsten is the top-level project).
- After adding a source or header, remember to list it in the `add_library`
  call in the top-level `CMakeLists.txt` — sources are listed explicitly, not
  globbed.

## Code conventions

Match the surrounding code. Established patterns:

- **File header:** every file starts with the standard copyright block
  (`Copyright © <year> Jan Erik Breimo`) and is licensed under the **Zero-Clause
  BSD License** — use exactly `This file is distributed under the Zero-Clause
  BSD License.` in the header of every new file.
- **Include guard:** `#pragma once`.
- **Namespace:** everything lives in `namespace Tungsten`. Internal-only helpers
  go under `Detail`.
- **Naming:** `snake_case` for functions, methods, and locals; `PascalCase` for
  types (classes, structs, enums); `UPPER_SNAKE_CASE` for enum values and
  constants. Member variables carry a trailing underscore (`name_`,
  `location_`). Prefer `enum class`.
- **Formatting:** 4-space indent, Allman braces (opening brace on its own line),
  one class/function per concept. Keep lines reasonably short.
- Build with warnings enabled (`Xyz_enable_all_warnings`); keep the tree
  warning-clean.

## OpenGL & error handling

- Do **not** call `gl*` functions directly. Go through the wrapper returned by
  `get_ogl_wrapper()` (the `IOglWrapper` abstraction), which also enables the
  Emscripten/dummy backends.
- Wrap GL calls that can fail with the error-checking macros from
  `TungstenException.hpp`: `THROW_IF_GL_ERROR()` after GL operations,
  `THROW_GL_ERROR(code)` / `TUNGSTEN_THROW(msg)` for explicit failures. All
  errors are reported by throwing `TungstenException`.
- Resource ownership uses RAII handle types (`BufferHandle`, `ProgramHandle`,
  etc.) with custom deleters — don't manage raw GL ids manually.

## Shaders

- Builtin shaders live in `src/Tungsten/Shaders/*.glsl` and are embedded at
  build time via **cppembed** (`#embed_text` in `ShaderSources.cpp.in`).
  Editing a `.glsl` file is enough; the generated `ShaderSources.cpp` rebuilds
  automatically. Never edit the generated file under `cmake-build-*/`.
- Target **GLSL ES 3.00** (`#version 300 es`) for WebGL/Emscripten
  compatibility. Guard precision qualifiers with `#ifdef GL_ES`.
- Prefer runtime control over compile-time permutations: switch lights and
  material features with `uniform bool`/count uniforms rather than `#ifdef`
  defines. Reserve `#define` for values that must be known at compile time
  (e.g. array sizes like `MAX_POINT_LIGHTS`). See `SmoothShader` /
  `BlinnPhong-*.glsl` for the reference pattern.
- C++ wiring for shaders lives in `ShaderPrograms/`: a `ShaderProgram` subclass
  plus `*Uniform` helper structs that bind uniform locations. Vertex attribute
  layout is exposed through `attribute_definitions()`; the stride is determined
  by what the *caller* adds, so only advertise attributes that are part of the
  default vertex format.

## Working agreements

- Verify changes by building the affected target(s) **and** the examples; a
  change to a shader or `ShaderProgram` can break `examples/cube` even when the
  library compiles.
- Don't commit, branch, or push unless asked.
- Keep public API changes reflected in the umbrella `Tungsten.hpp` when adding
  a new public header.