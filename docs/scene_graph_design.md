# Scene graph — design notes

Companion to `docs/diagrams/src/scene_graph.mermaid`. The class diagram shows the shape of
the design; this document records the decisions and constraints that the diagram cannot
express. The exploratory discussion that led here is in `scene_graph_chat.md`.

## 1. Context

The diagram's structure is sound: a four-way split (scene / bridge / resource / render) with
an immutable `RenderSnapshot` as the only contract between the mutable scene graph and the
renderer, and a resource layer built on shared (sub-allocated) GL buffers. Three things
needed correcting:

1. **Dirty propagation was modelled naively.** The world transform had no home and the
   updater did an eager full-tree walk.
2. **It assumed desktop OpenGL 4.x.** Tungsten also targets WebGL2 / GLSL ES 3.00 via
   Emscripten, where several of the assumed features do not exist.
3. **Pieces were missing or under-specified:** resource handles, `LightData`, `VertexLayout`,
   frustum culling, the 2D path, and double-buffering.

## 2. Dirty-propagation flow

World transforms are resolved **lazily and only where something changed**, never by
recomputing the whole tree each frame.

State per `Node`:

- `localDirty` — set when the local `Transform` changes.
- `worldMatrix` — cached world transform.
- `worldVersion` — bumped every time `worldMatrix` is recomputed.
- `parentVersionSeen` — the parent's `worldVersion` at the time this node last recomputed.

`world_matrix()` recomputes iff:

```
localDirty || parent == nullptr ? false : parent.worldVersion != parentVersionSeen
```

On recompute: `worldMatrix = parent.world_matrix() * localTransform.local_matrix()`, then
bump `worldVersion`, store the parent's current `worldVersion` into `parentVersionSeen`, and
clear `localDirty`.

**Reparenting is correct for free.** Detaching and re-attaching a node elsewhere does not
touch the node's own data, but the new parent has a different `worldVersion`, so the version
mismatch forces a recompute on next access — no special-casing needed. (A plain "subtree
dirty" bool would miss this, which is the classic reparenting bug.)

`TransformUpdater::resolve(scene)` walks dirty subtrees once before extraction so that
`SnapshotBuilder` reads finalized, order-independent world matrices rather than triggering
lazy recompute mid-traversal.

**Two directions in one pass.** Transforms propagate **down** (a parent's change invalidates
children). Aggregate bounds propagate **up** (`propagate_bounds`): a node's `worldBounds` is
its own renderable bounds unioned with its children's world bounds, so a moved child grows
its ancestors' bounds. Keeping both directions in mind is what makes hierarchical frustum
culling possible later.

## 3. WebGL2 / GLES 3.00 constraints

The portable path must run on WebGL2 (= GLES 3.0). Features assumed by the original sketch
that are **not** available there, and what we do instead:

| Desktop feature | WebGL2 status | Portable approach |
| --- | --- | --- |
| `glDrawElementsBaseVertex` | Absent (GLES 3.2) | **Rebase indices to absolute offsets at upload.** No `baseVertex` field in the core `Mesh`; a base-vertex fast path is a desktop-only optimization, not part of the contract. |
| SSBO + `gl_DrawID` per-draw indexing | Absent (GLES 3.1+) | Per-draw data via a per-draw uniform, or an **instanced vertex attribute** (`vertexAttribDivisor`, which *is* in GLES 3.0). |
| Multi-draw indirect | Absent | One draw call per `RenderItem`; rely on sort-key batching to minimize state changes. |
| UBOs | **Available** | Used for per-frame and per-material data (see §4). |
| 32-bit indices | Available in WebGL2 | `IndexType` allows `UNSIGNED_SHORT` or `UNSIGNED_INT`; prefer short where the mesh fits. |

Two repo rules reinforced in the design:

- **No raw `gl*` calls.** Everything goes through `IOglWrapper` / `get_ogl_wrapper()`.
- **No raw GL ids on owned resources.** The diagram uses Tungsten's existing RAII handle
  types — `BufferHandle`, `ProgramHandle`, `TextureHandle`, `VertexArrayHandle` (from
  `include/Tungsten/Gl/GlHandle.hpp`) — not `GLuint`.

## 4. UBO binding convention

Binding points are fixed across **all** shaders so the renderer never branches on shader
type:

- **binding 0 — per-frame**: camera matrices, time, lights. Bound once per frame.
- **binding 1 — per-material**: the `Material::parameterData` blob. Bound on material change.
- **binding 2 — per-draw**: world / normal matrix (or supplied as an instanced attribute).

This convention is enforced by reflecting each program's interface at load and validating it,
or by documented contract for hand-written shaders.

## 5. Double-buffering

`Scene` owns a `DoubleBuffer<RenderSnapshot>`. Each frame, single-threaded:

```
update scene (animation, etc.)
TransformUpdater::resolve(scene)
SnapshotBuilder::build(scene, camera, snapshots.back())
snapshots.swap()
Renderer::render(snapshots.front())
```

The swap is a pointer/index flip — no locks, no fences in the single-threaded case. The two
snapshots reuse their backing `std::vector` storage across frames rather than reallocating.

This runs fine single-threaded today (the only mode on WebGL2/Emscripten, which is one
context on one thread). Its value here is that the renderer never observes half-built state,
and the "renderer only reads `front()`" contract is exactly the seam where a render thread
would later add synchronization — adopting it now costs nothing on the web target.

## 6. Resource references

Logical resource references (`MeshRef`, `MaterialRef`, `ShaderProgramRef`, `TextureRef`,
`BufferArenaRef`) are `ResourceRef<T>{ uint32 index, uint32 generation }`. The generation is
bumped when a slot is freed, so a stale ref (held across a deferred deletion + slot reuse)
fails validation on lookup instead of silently aliasing a different resource. A ref is
therefore **revocable** — it is a key into `ResourceManager`'s slot table, not a stable id, and
must not be persisted as if it were one.

## 7. Buffer arenas and sub-allocated buffers

GL buffers are not allocated one-per-mesh. A `BufferArena` owns **one** GL
`BufferHandle` and hands out sub-ranges of it; many meshes share one buffer. This
is the resource-layer mechanism behind `VertexStream::vbo` and `Mesh::ebo`.

**`SharedBuffer` is a non-owning slice, not shared ownership.** It is a plain
value `{ BufferArenaRef arena, uint32 offset, uint32 count }` — trivially
copyable, no refcount. The name refers to the *buffer being shared* among
allocations, not to shared ownership of the GL buffer.

Resolving `arena` → `BufferArena` → `BufferHandle` goes through
`ResourceManager::get_arena(BufferArenaRef)`. This happens at **VAO build
time** (`get_vao` bakes the VBO and element-buffer bindings into the VAO) and at
**allocate / upload / free** time — both inside `ResourceManager`. It does *not*
happen per draw: the renderer binds the mesh's VAO and issues the draw with
`offset`/`count`, because the buffer bindings are already VAO state. The arena
ref is generational like the other resource refs in §6, so a stale slice
fails validation rather than aliasing a regrown buffer.

**VAO identity follows the buffer pairing.** Because the element-array binding is
VAO state (not a per-draw argument), a VAO is valid only for one specific
(VBO arena, EBO arena) pairing — `get_vao` bakes both in. Two meshes drawn from
the same pair of arenas share a VAO; meshes from different arenas need different
VAOs. The per-stride-arena rule above keeps this stable: a mesh stays within its
arenas, and the offset-preserving growth means the only time the bindings change
is when an arena reallocates its `BufferHandle`, at which point the affected VAOs
are rebuilt anyway.

(If a slice ever genuinely needed to keep its storage alive by itself — it does
not today, the arena does — the unit to share would be the *arena*, not the bare
handle: `shared_ptr<BufferArena>` with a deleter that returns the range to the
free-list. Never `shared_ptr<BufferHandle>`.)

**The allocator is `Detail::BuddyAllocator`, already in the tree.** A
`BufferArena` is `BufferHandle` + `BuddyAllocator` (offset management) + growth
logic. This pattern is not new: `TextRenderer` already pairs a GL buffer with two
`BuddyAllocator`s and grows by re-`claim()`ing every live block into a larger
allocator (`src/Tungsten/Render/TextRenderer.cpp`). `BufferArena` factors that out
into a named, reusable type. `allocate()` wraps the offset `BuddyAllocator` returns
in a `SharedBuffer`; `free()` calls `BuddyAllocator::free`; `grow()` reallocates
the GL buffer and replays `claim()` for every live allocation.

Two decisions this commits to:

- **Buddy's 2× internal fragmentation is accepted.** `BuddyAllocator` rounds each
  request up to a power of two, so up to half an allocation's space can be wasted.
  That is ideal for many small, similar allocations (the text path) and cheap to
  reuse here. For arenas holding a few large, oddly-sized meshes it can waste real
  VBO space; if that shows up in profiling, switch those arenas to a best-fit
  free-list allocator behind the same `BufferArena` interface. Start with buddy.
- **One arena per stride, allocating in vertex/index units.** Then `offset` is
  directly `VertexStream::firstVertex` and `count` is `vertexCount`, buddy's
  natural power-of-two alignment subsumes any explicit alignment field, and VAO
  binding stays simple. A single arena mixing strides would instead allocate in
  bytes and make stride/alignment do real work; the per-stride split avoids that.

**Growth preserves offsets.** Doubling a buddy allocator's capacity leaves every
live block at its original offset (the "claim after doubling" test in
`tests/TungstenTest/test_BuddyAllocator.cpp` checks exactly this). So when an arena
grows, existing `SharedBuffer{offset,count}` values stay valid — only the GL
`BufferHandle` is rebound — and the immutable `RenderSnapshot` never needs its
meshes re-patched.

## 8. 2D / 3D unification

2D and 3D share one `Node`/`Transform`, the whole resource and material layer, the snapshot,
and the renderer. The only branch is the camera: `CameraComponent::mode` selects
`PERSPECTIVE` (3D, uses `fov`/`near`/`far`) or `ORTHOGRAPHIC` (2D, uses `orthoSize` and
aspect). A 2D scene is just nodes at `z = 0` with rotation about the z axis, viewed through an
orthographic camera; draw order for 2D is handled through the existing render-layer / sort-key
machinery rather than a separate code path.

## 9. Integration with existing Tungsten

This is a forward-looking design; current code migrates toward it rather than being replaced
wholesale:

- **`MeshData`** (`MeshDataBuilder.hpp`, `MeshUtilities.hpp`) is the current CPU-side mesh
  container. The new `Mesh` + `VertexStream` + `BufferArena` are its GPU-resident,
  sub-allocated successor; `MeshData` becomes the source that gets uploaded into an arena.
- The existing typed `ShaderProgram` subclasses + `*Uniform` helper structs in
  `ShaderPrograms/` are replaced on this path by the generic `Material` parameter blob +
  fixed UBO conventions, so the renderer stays shader-agnostic. The typed helpers can remain
  as a convenience layer for constructing `Material::parameterData`.
- All GL access continues through `IOglWrapper`, and resources use the existing RAII handle
  types, so the design stays within the repo's established conventions.
