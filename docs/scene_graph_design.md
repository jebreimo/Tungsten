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
into a named, reusable type. `allocate()` returns the byte `offset` the
`BuddyAllocator` hands out (converted from units via the arena's stride); `free()`
takes that byte offset and calls `BuddyAllocator::free`; `grow()` reallocates the
GL buffer and replays `claim()` for every live allocation.

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

**The arena deals in byte offsets; `ResourceManager` owns identity.** A
`BufferArena` does **not** know its own `BufferArenaRef` — that `{index,
generation}` is `ResourceManager`'s slot bookkeeping, and `generation` is
authoritative there. Rather than hand out a half-formed slice it cannot fully
construct, the arena does not traffic in `SharedBuffer` at all: `BufferArena::allocate`
returns an `Allocation{ uint32 offset, retired_buffer }` (a bare byte offset), and
`ResourceManager::allocate(BufferArenaRef, count)` — the one caller that knows the
ref — pairs that offset with the ref and the count to form the `SharedBuffer` in one
shot. This keeps the dependency one-directional (`ResourceManager` → `BufferArena`,
never the reverse), cleanly splits the two axes (the arena owns **range** /
offset-and-stride; `ResourceManager` owns **identity** / the ref), and makes the
same `ResourceManager` boundary the place that also (a) retires the displaced GL
buffer returned by a grow and (b) rebuilds the VAOs invalidated when the arena's
buffer id moves. `free` is symmetric: `ResourceManager` resolves `slice.arena`
through `get_arena` and forwards `slice.offset` to the arena. `SharedBuffer` is thus
purely a `ResourceManager`-level type; `BufferArena` does not include its header.
See the sketch in `src/Tungsten/Neo/ResourceManager.hpp`.

Because cloning to grow changes the buffer id, the old buffer can still be in use
by in-flight draws (or, with a render thread, by the snapshot being rendered). It
is therefore moved onto a frame-tagged retirement queue and freed only once a
completed frame (single-threaded: the just-drawn frame; threaded: the latest
passed fence) proves nothing references it — the deferred-deletion half of the
generational scheme in §6.

**The VAO cache lives in `ResourceManager`, not in `Mesh`.** Since a VAO is shared
by every mesh with the same `(vbo arenas, ebo arena, layout)` combination, no
single `Mesh` can *own* it. `ResourceManager` owns the cache; `get_vao(vboArenas,
eboArena, layout)` returns a **non-owning VAO id (`uint32`)**, and a `Mesh` stores
that id — not a `VertexArrayHandle`. (This corrects the earlier diagram, where
`Mesh` held an owning `VertexArrayHandle`.) Two consequences:

- **The cache is keyed on arena *refs*, not live buffer ids.** A grow changes an
  arena's buffer id but not which arena a mesh draws from, so keying on the
  `BufferArenaRef`s keeps the entry valid across growth. `rebuild_vaos_for_arena`
  then walks the cache, and for every VAO whose key references the grown arena,
  re-points its baked-in buffer binding at the arena's new id **in place** — the
  VAO id is unchanged, so every `Mesh` holding it stays valid and nothing in the
  snapshot is patched. This is why growth never needs to retire VAOs, only the old
  buffer.
- **Layouts are referenced by `VertexLayoutRef`**, the same generational-ref
  family as the other resources (§6). Interning layouts gives them a small,
  comparable identity that forms part of the VAO key without the cache needing the
  full `VertexLayout` value.

See the sketch in `src/Tungsten/Neo/ResourceManager.{hpp,cpp}`.

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

## 10. Resource manager decomposition

The naïve completion of the resource side — one bespoke slot table per type plus VAO, shader,
and deletion logic all inlined — makes `ResourceManager` a ~30-method god object with the same
slot code written five times. The design instead makes `ResourceManager` a **thin facade** over
one reusable container and three focused collaborators it owns:

- `GenerationalPool<T>` — the slot-table machinery, instantiated once per resource type (§10.1).
- `DeletionQueue` — frame-tagged deferred deletion of all GL objects (§11).
- `LayoutRegistry` — VertexLayout interning (§12).
- `VaoCache` — the shared-VAO cache and its baking (§13).
- `ShaderLibrary` — shader families and variant compilation (§14).

The facade owns these, wires them together (the pools and caches retire into the one
`DeletionQueue`; `VaoCache` resolves refs through the arena pool and `LayoutRegistry`), and
exposes them behind **forwarding methods** — `create_mesh` / `get_mesh` / `destroy_mesh`,
`get_arena`, `register_shader_variant`, `get_vao`, `begin_frame`, `collect_garbage`, and so on.
The public surface therefore looks as it did before, but each method is a one-liner delegating
to a collaborator; the substance lives in independently testable pieces. `SnapshotBuilder` and
`Renderer` still talk to one object.

### 10.1 `GenerationalPool<T>`

Every owned resource type (`Mesh`, `Material`, `ShaderProgram`, `Texture`, `BufferArena`) is a
`GenerationalPool<T>` — the `ArenaSlot` pattern generalized once. A slot is
`{ std::optional<T> value; uint32 generation; }` (empty `value` marks a free slot), backed by a
free-list of indices. The pool has exactly three operations:

- `insert(T&&) -> ResourceRef<T>` moves a fully-built resource into a free slot (reused from the
  free-list, or appended) and returns `{ index, slot.generation }`.
- `get(ResourceRef<T>) -> T&` validates the ref — index in range **and** `slot.generation ==
  ref.generation` **and** `value` non-empty — throwing `TungstenException` otherwise. This is
  the old `get_arena` logic, written once.
- `erase(ResourceRef<T>, on_retire)` empties the slot, **bumps its generation** (revoking every
  outstanding ref), and returns the index to the free-list. It does **not** delete GL objects
  itself: it invokes the `on_retire` callback with the departing value so the caller can move
  the value's GL handles onto the `DeletionQueue`. This callback seam is what keeps the pool
  ignorant of both GL and frames while still driving deferred deletion.

The facade's `create_* / get_* / destroy_*` forward straight to `insert / get / erase`. Two
resource types keep a richer *create* path in front of `insert`: `BufferArena` because the
manager also constructs and grows it (`create_arena(usage, stride, capacity)`), and
`ShaderProgram` because it is only ever produced by `register_shader_variant` (§14), never
inserted directly by callers.

**Why the generation bump and deferred deletion are orthogonal.** The generation bump makes a
*stale ref* fail lookup instead of aliasing a reused slot — a CPU-side correctness guard. The
`DeletionQueue` keeps the *GL object* alive until no in-flight frame can still read it — a
GPU-side lifetime guard. They solve different problems, so a slot's index may be reused on the
very next `insert` (the fresh resource gets its own new GL object) while the destroyed
resource's old GL object is still draining through the queue. The renderer never touches a
stale ref because snapshots are rebuilt from scratch each frame (§5): a destroyed resource
simply stops appearing in new snapshots, and the last snapshot that named it is retired by the
same frame accounting that retires the GL object.

## 11. Deferred deletion — the `DeletionQueue`

The deferred-deletion machinery is factored into one collaborator, `DeletionQueue`, that the
facade owns and every other piece retires into. §7 retires the *buffer* displaced by an arena
grow; the same queue covers **every** GL object taken out of service — a grown arena's old
`BufferHandle`, a destroyed resource's `ProgramHandle` / `TextureHandle` (moved out by the
pool's `erase` callback, §10.1), and a `VertexArrayHandle` evicted from the `VaoCache` (§13).

- `begin_frame(frame)` records the id of the frame about to be built and submitted; everything
  retired until the next call is tagged with it.
- `retire(handle)` pushes an owning RAII handle onto the queue paired with the current frame.
  The object stays alive because the queue still owns its handle.
- `collect_garbage(completed_frame)` drops (and thereby deletes, via the handle destructor)
  every entry whose tag is `<= completed_frame`. Single-threaded, `completed_frame` is the
  just-drawn frame; with a render thread it is whatever the latest passed fence reports.

Because retired objects are heterogeneous (buffer / program / texture / VAO), the queue holds a
small owning variant (or one queue per handle type — an implementation choice, not a design
one). This subsumes the sketch's `retired_buffers_`. The facade's `begin_frame` /
`collect_garbage` are forwarders to the queue's.

*Naming note:* the diagram's `process_deletions()` / `deferred_deletion_queue` are this
collaborator's `collect_garbage` / the queue itself; the code uses the `begin_frame` /
`collect_garbage` pair because it makes the frame-tagging explicit.

## 12. VertexLayout interning — the `LayoutRegistry`

§7 states layouts are interned and referred to by `VertexLayoutRef`; the `LayoutRegistry`
collaborator is the mechanism, reached through the facade's `register_layout` / `get_layout`.

- `register_layout(const VertexLayout&) -> VertexLayoutRef` scans the intern table for a value
  equal to the argument (`VertexLayout::operator==` is defaulted, deep-comparing attributes and
  stride) and returns the existing ref if found, otherwise appends and returns a new one.
- `get_layout(VertexLayoutRef) -> const VertexLayout&` resolves a ref for VAO building.

**Layouts are never individually freed**, so the registry is a plain interning vector, *not* a
`GenerationalPool` — it needs no free-list, no generations, and no deletion path. The set of
distinct vertex formats an application uses is small, bounded, and owns no GL object, so
interning them for the manager's lifetime costs almost nothing and buys two things: a
`VertexLayoutRef` is a *permanently* stable key (no generation churn), so it can serve as part
of the `VaoCache` key (§13) and be embedded in a `RenderItem`'s sort key without revocation
concerns. Consequently a layout ref's `generation` is a constant; only the index carries
information.

## 13. Attribute-location convention and the `VaoCache`

The shared-VAO cache is a collaborator, `VaoCache`, reached through the facade's `get_vao`. It
owns the cache entries and the baking logic; it depends only on a resolver for arenas and
layouts (the arena pool + `LayoutRegistry`) and on the `DeletionQueue`. This section records
the convention and steps it bakes.

**Attribute locations are a fixed convention keyed on `AttributeSemantic`, not per-shader.**
A cached VAO is shared by *every* mesh with a given `(vbo arenas, ebo arena, layout)` triple
(§7) **regardless of which shader draws it** — the VAO cache key has no shader in it. So the
attribute *locations* baked into a VAO cannot depend on a shader; they must be a global
convention. We define

```
attribute_location(semantic) = static_cast<uint32_t>(semantic)
```

i.e. the `AttributeSemantic` enum order *is* the location table (`POSITION = 0`, `NORMAL = 1`,
…). This is the vertex-attribute analogue of the fixed UBO binding points in §4: every shader
declares `layout(location = N) in …` to match, and the renderer never remaps. `ShaderProgram::
required_layout` exists to **validate** this at load (catch a shader whose declared inputs
disagree with the mesh format), not to relocate attributes.

**`build_vao(key)` bakes, per the resolved layout:**

1. For each vertex stream *i* (each entry of `key.vbo_arenas`): bind that arena's current
   buffer to `GL_ARRAY_BUFFER`, then for every `VertexAttribute` with `stream_index == i`
   call `define_vertex_attribute_pointer(attribute_location(semantic), component_count,
   data_type, stride, offset_in_stream, normalized)` and enable it. **The per-vertex stride is
   the arena's `stride()`**, not `layout.stride` or `VertexStream::stride`: the one-arena-per-
   stride rule (§7) makes the arena the single authority on a stream's byte pitch, and it is
   the value available at build time (the cache key holds arena refs, not streams).
2. Bind the element buffer: `bind_buffer(ELEMENT_ARRAY_BUFFER, get_arena(ebo_arena).
   buffer_id())`. This binding *is* VAO state, which is exactly why the VAO is specific to one
   EBO arena.

Per-mesh base offsets (`first_vertex`, `first_index`) are **not** baked — array draws pass
`first`, indexed draws use rebased absolute indices (§3) — so meshes differing only by offset
reuse one VAO.

**Rebuild on grow (`rebuild_vaos_for_arena`) re-points, it does not recreate.** WebGL2 / GLES
3.00 has no separate vertex-buffer binding points (`glBindVertexBuffer` is GL 4.3), so a VAO's
attributes are tied to whatever buffer was bound to `GL_ARRAY_BUFFER` when
`define_vertex_attribute_pointer` ran. "Re-pointing" therefore means: bind the VAO, bind the
arena's *new* buffer, and re-run the attribute-pointer calls for the streams drawn from that
arena (and/or re-bind the element buffer if the grown arena is the EBO arena). The VAO id is
unchanged, so every `Mesh` holding it stays valid and no snapshot is patched.

**Cache eviction is driven by arena lifetime.** Destroying an arena (§10) is only valid once
no live mesh draws from it; at that point every cached VAO whose key references the arena is
removed and its `VertexArrayHandle` retired through the deferred queue (§11). Growth never
evicts — it rebuilds in place. Because the cache is keyed structurally on refs and cleaned by
arena lifetime, VAOs need no reference counting.

## 14. Shader families and variant compilation — the `ShaderLibrary`

Shader compilation is factored into the `ShaderLibrary` collaborator, reached through the
facade's `register_shader_family` / `register_shader_variant`. It owns the family registry and
the variant cache, and it is the *only* producer of `ShaderProgram` values — it inserts them
into the shader pool (§10.1) and hands back a `ShaderProgramRef`, so callers never build a
`ShaderProgram` directly. Its job is a compile-or-fetch that keeps the *count* of compiled
programs bounded even though real shaders have many feature permutations (skinning,
normal-mapping, alpha-clip, …). See `scene_graph_chat.md` §"Layer 5".

- A **shader family** is a named GLSL source pair (vertex + fragment) plus the ordered list of
  feature flags it understands and the `VertexLayout` its attributes expect.
  `register_shader_family(ShaderFamilyId, sources, features, required_layout)` records one; the
  `ShaderFamilyId` is an interned family name. Builtin families come from the embedded
  `Shaders/*.glsl` sources (cppembed), so this stays within the existing shader pipeline.
- `register_shader_variant(ShaderVariantKey{ family, defines })` looks the key up in the variant
  cache (keyed by value equality on the key). **Hit:** return the cached `ShaderProgramRef`.
  **Miss:** expand `defines` — bit *i* set ⇒ prepend `#define <features[i]>` to the family
  source — compile and link, assign the program a stable numeric `id` (packed into
  `RenderItem::sort_key` so same-program items batch), populate the `ShaderProgram`
  (`gl_handle`, `id`, `variant_key`, `required_layout`), insert it into the shader pool, record
  `key -> ref` in the cache, and return the ref.

The `defines` bitmask, not a string set, is what makes the key cheap to compare and hash; the
family's ordered feature list is the single place that maps a bit to its `#define` spelling.
`Material` selects appearance by holding the resolved `ShaderProgramRef`; the
`SnapshotBuilder` (or material-authoring code) is what turns "this material has a normal map
and is skinned" into a `ShaderVariantKey` and resolves it here.
