# Scene graph — design notes

Companion to `docs/diagrams/src/scene_graph.mermaid`. The class diagram shows the shape of
the design; this document records the decisions and constraints that the diagram cannot
express. The exploratory discussion that led here is in `scene_graph_chat.md`.

## 1. Context

The diagram's structure is sound: a four-way split (scene / bridge / resource / render) with
an immutable `RenderSnapshot` as the only contract between the mutable scene graph and the
renderer, and a resource layer built on shared (sub-allocated) GL buffers. Three things
needed correcting:

1. **The world transform had no home.** Where it was cached, and what made it stale, was
   left unsaid. §2 answers that — and has since been rewritten a second time, replacing
   per-node dirty flags with flat arrays and one linear pass.
2. **It assumed desktop OpenGL 4.x.** Tungsten also targets WebGL2 / GLSL ES 3.00 via
   Emscripten, where several of the assumed features do not exist.
3. **Pieces were missing or under-specified:** resource handles, `LightData`, `VertexLayout`,
   frustum culling, the 2D path, and double-buffering.

## 2. Flat node storage and the transform pass

**There is no `Node` class.** `Scene` owns one flat array per node attribute, all indexed by
`NodeId::index`, and a node *is* that index. Nothing is allocated per node and nothing holds
a pointer to one, so every per-frame pass is a loop over contiguous memory instead of a walk
through separately allocated tree nodes.

The arrays:

| Array | Purpose |
| --- | --- |
| `generations` | revokes ids to a slot's previous occupant (§6) |
| `parents`, `firstChildren`, `nextSiblings` | the topology, as links rather than child vectors |
| `locals` | each node's `Transform` |
| `worlds` | each node's resolved world matrix |
| `order` | every live node, parents before children |

**Identity is a `NodeId`, not an address.** `NodeId` is `ResourceRef<NodeTag>` — the same
`{index, generation}` revocable key the resource layer uses (§6) — so removing a node bumps
its generation and every outstanding id to it fails validation instead of silently naming
whichever node reuses the slot. `NodeHandle` is the ergonomic wrapper: a copyable
`{Scene*, NodeId}` pair that turns `scene.set_local_transform(id, t)` back into
`node.set_local_transform(t)`. It is a value, not a reference — nothing in the scene points
back at it.

**The hierarchy is first-child / next-sibling links**, not a child vector per node. That
keeps the topology in the same flat arrays as everything else and makes structural edits
O(1) rather than a vector splice. Roots are linked through the same sibling chain, so they
need no special case. `children()` returns a range that walks the chain and allocates
nothing.

**`Transform` is a plain value** — translation, rotation (a quaternion), scale, with
`make_matrix()` computed on demand. It carries no cache and no dirty flag.

**Resolution is one linear pass, not a lazy recompute.**

```cpp
for (const uint32_t index : order_)
{
    const NodeId parent = parents_[index];
    worlds_[index] = parent ? worlds_[parent.index] * locals_[index].make_matrix()
                            : locals_[index].make_matrix();
}
```

`order_` lists every live node with parents before children, which is the entire correctness
argument: by the time the loop reaches a node, `worlds_[parent]` is already final. It is
rebuilt only when the hierarchy actually changed — structural edits set `hierarchyDirty`, and
`resolve_transforms()` clears it. The rebuild appends the roots and then walks the output
vector itself as a queue, appending each node's children as it passes over them; visiting
breadth-first gives the parents-before-children invariant for free and keeps siblings in
insertion order.

This replaces an earlier scheme of per-node `localDirty` / `worldVersion` /
`parentVersionSeen` counters that recomputed lazily on access. That scheme skipped unchanged
subtrees, but it cost three words per node, a subtle reparenting special case, and a
pointer-chasing traversal — and the pass it was avoiding is a sequential walk over two arrays.
Skipping work is only a win when the work is more expensive than deciding to skip it.

**World matrices are only current after a resolve.** `world_matrix(id)` returns what the last
`resolve_transforms()` computed; it does not recompute on access. A node reads as unmoved
until the first resolve.

**Components are typed arrays, not a polymorphic list.** `Scene` holds one
`ComponentStore<T>` per kind — `items` and a parallel `owners` of `NodeId` — for
`RenderableComponent`, `LightComponent` and `CameraComponent`. The components are plain
aggregates: no base class, no virtual destructor, no `owner` back-pointer. Attachment is
`handle.add<T>(...)`, lookup is `get<T>()` / `find<T>()`, removal is a swap-and-pop, and
removing a node drops the components of its whole subtree by sweeping each store for owners
that are no longer alive.

The set of kinds is therefore **closed**: a new kind means a new array. That is the deliberate
trade. It removes the `dynamic_cast` chain that used to sit inside both per-frame traversals,
and it means the extraction pass iterates one contiguous array of a known type. User-defined
component types are not supported; if they are wanted later, the retrofit is a type-erased
`ComponentStore<T>` registry keyed on `type_index`, which nothing here forecloses.

**Bounds do not propagate up.** An earlier version maintained an aggregate `worldBounds` per
node for a hierarchical subtree cull. That is gone — see §15 for why, and for what culling
does instead.

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
- **binding 1 — per-material**: the material's own parameter buffer. Re-pointed on material
  change.
- **binding 2 — per-draw**: world / normal matrix, bound as a range of one packed buffer
  (or supplied as an instanced attribute).

GLSL ES 3.00 cannot declare binding points in the shader source, so the `ShaderLibrary`
assigns them right after linking each variant: it looks up the conventional block names
(`PerFrame`, `MaterialBlock`, `PerDraw`) and binds whichever of them the program declares.
Hand-written shaders participate by using those block names.

**Binding 0 is the renderer's own buffer.** It owns one, binds it at startup, and afterwards
only rewrites its contents, once per frame. The buffer there never changes, so the bind is
not worth caching.

**Binding 2 is one packed buffer, bound by range.** The renderer sorts both passes up front,
so before any drawing it knows every item that will be drawn and in what order. It writes
each item's block into one staging array — spaced by `GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT`,
since every `glBindBufferRange` offset must be a multiple of it — uploads that once, and then
each draw only points binding 2 at its own slice.

The alternative is what this replaced: `glBufferData` into a single small buffer before every
draw, which is N buffer respecifications per frame against N cheap range binds plus one
upload. The cost is the padding between blocks — a 112-byte block spaced at the 256 bytes a
typical driver demands wastes 56%, which is 256 KB for a thousand items and therefore not
worth thinking about. Note the upload respecifies the whole store rather than sub-updating
it: handing the driver a fresh allocation each frame is the cheap way to avoid writing into
memory the GPU is still reading, absent explicit fences. A true multi-frame ring buffer with
`glFenceSync` would go further, and is available in GLES 3.0 if it is ever needed.

**Binding 1 works the other way round.** Each `Material` owns the buffer holding its
parameters, uploaded once by `ResourceManager::create_material` (or again on
`update_material_parameters`). Drawing a material only re-points binding 1 at that buffer, so
a material's parameters are never re-uploaded just because it was drawn again. This is the
bind `GlStateCache` exists for: the sort key groups items by material, so consecutive items
sharing one bind nothing at all.

Uploading per material switch instead would repeat the same bytes every frame for data that
essentially never changes — and it would make a material with an empty blob draw against
whatever the previous material had left in the shared buffer. With per-material buffers that
state cannot arise: a material either has a buffer or it does not, and a shader that declares
`MaterialBlock` against a material that has none is rejected outright (`ShaderProgram::
has_material_block`).

**Texture units follow the same fixed-convention idea.** A `ShaderFamily` lists its sampler
uniforms in unit order (`samplers`), and the `ShaderLibrary` points sampler *i* at texture
unit *i* right after linking each variant — samplers cannot live in UBOs, and their GLSL
default (unit 0 for every sampler) is never what a multi-textured material wants. The
renderer binds `Material::textures[i]` to unit *i*, so the two orders meet in the middle.
Units the program samples but the material leaves unfilled get the renderer's 1×1 white
dummy texture: a sampler must always see a complete texture (macOS's GL driver warns
otherwise), and white is the multiplicative identity if the shader reads it anyway.

## 5. Double-buffering

`Scene` owns a `DoubleBuffer<RenderSnapshot>`. Each frame, single-threaded:

```
update scene (animation, etc.)
scene.resolve_transforms()
SnapshotBuilder::build(scene, camera_node, snapshots.back())
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
is the resource-layer mechanism behind the `SharedBuffer` slices a `Mesh` draws
from — its vertex streams and its `ebo`.

**`SharedBuffer` is a non-owning slice, not shared ownership.** It is a plain
value `{ BufferArenaRef arena, uint32 offset, uint32 count }` — trivially
copyable, no refcount. The name refers to the *buffer being shared* among
allocations, not to shared ownership of the GL buffer. `offset` and `count` are
in the arena's stride units (vertices or indices), never bytes; byte offsets
exist only where a GL call needs one, computed via the arena's `stride()`.

**A `Mesh`'s vertex streams are plain `SharedBuffer`s.** Because the arena
allocates in vertex units, a slice's
`offset` *is* the base vertex a draw passes as `first` (or folds into rebased
indices) and its `count` *is* the vertex count; a per-stream stride field would
only duplicate the arena's `stride()`, which is the single authority on a
stream's byte pitch (see §13). The same identity covers the index side: the
`ebo` slice's `offset` and `count` are the first index and index count, so
`Mesh` is just `{ vao, streams, layout, ebo, index_type, primitive }`.

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
into a named, reusable type. `allocate()` returns the unit `offset` the
`BuddyAllocator` hands out, or `nullopt` when the arena is full — it never grows
on its own (see below); `free()` takes that offset back to `BuddyAllocator::free`;
`grow()` reallocates the GL buffer and replays `claim()` for every live
allocation.

Two decisions this commits to:

- **Buddy's 2× internal fragmentation is accepted.** `BuddyAllocator` rounds each
  request up to a power of two, so up to half an allocation's space can be wasted.
  That is ideal for many small, similar allocations (the text path) and cheap to
  reuse here. For arenas holding a few large, oddly-sized meshes it can waste real
  VBO space; if that shows up in profiling, switch those arenas to a best-fit
  free-list allocator behind the same `BufferArena` interface. Start with buddy.
- **One arena per stride, allocating in vertex/index units.** Then a slice's
  `offset` is directly the base vertex/index a draw uses and its `count` the
  vertex/index count (which is what lets `Mesh` use bare `SharedBuffer`s as
  streams), buddy's natural power-of-two alignment subsumes any explicit
  alignment field, and VAO binding stays simple. A single arena mixing strides
  would instead allocate in bytes and make stride/alignment do real work; the
  per-stride split avoids that.

**Growth preserves offsets.** Doubling a buddy allocator's capacity leaves every
live block at its original offset (the "claim after doubling" test in
`tests/TungstenTest/test_BuddyAllocator.cpp` checks exactly this). So when an arena
grows, existing `SharedBuffer{offset,count}` values stay valid — only the GL
`BufferHandle` is rebound — and the immutable `RenderSnapshot` never needs its
meshes re-patched.

**The arena deals in bare offsets; `ResourceManager` owns identity and growth
policy.** A `BufferArena` does **not** know its own `BufferArenaRef` — that
`{index, generation}` is `ResourceManager`'s slot bookkeeping, and `generation` is
authoritative there. Rather than hand out a half-formed slice it cannot fully
construct, the arena does not traffic in `SharedBuffer` at all:
`BufferArena::allocate(count)` returns `optional<uint32> offset` (a bare unit
offset, `nullopt` when full), and `ResourceManager::allocate(BufferArenaRef,
count)` — the one caller that knows the ref — pairs that offset with the ref and
the count to form the `SharedBuffer` in one shot. When the arena is full, the same
method decides how much to grow, calls `grow()` — which returns the displaced
`BufferHandle` — and retries the allocation. This keeps the dependency
one-directional (`ResourceManager` → `BufferArena`, never the reverse), cleanly
splits the two axes (the arena owns **range** / offset-and-stride;
`ResourceManager` owns **identity** / the ref), and makes the same
`ResourceManager` boundary the place that also (a) retires the displaced GL
buffer a grow returns and (b) rebuilds the VAOs invalidated when the arena's
buffer id moves. `free` is symmetric: `ResourceManager` resolves `slice.arena`
through `get_arena` and forwards `slice.offset` to the arena. `SharedBuffer` is thus
purely a `ResourceManager`-level type; `BufferArena` does not include its header.
See `src/Tungsten/Neo/ResourceManager.hpp`.

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

See `src/Tungsten/Neo/ResourceManager.{hpp,cpp}`.

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

Everything downstream holds the ref, never a layout value: `Mesh::layout`,
`ShaderProgram::required_layout`, and `ShaderFamily::required_layout` are all
`VertexLayoutRef`s. Besides avoiding deep copies of interned data, this makes the load-time
layout validation in §13 a single ref comparison — interning guarantees equal layouts share a
ref.

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
disagree with the mesh format), not to relocate attributes — and since it is a
`VertexLayoutRef` (§12), the check is a ref comparison against the mesh's `layout`.

**`build_vao(key)` bakes, per the resolved layout:**

1. For each vertex stream *i* (each entry of `key.vbo_arenas`): bind that arena's current
   buffer to `GL_ARRAY_BUFFER`, then for every `VertexAttribute` with `stream_index == i`
   call `define_vertex_attribute_pointer(attribute_location(semantic), component_count,
   data_type, stride, offset_in_stream, normalized)` and enable it. **The per-vertex stride is
   the arena's `stride()`**, not `layout.stride`: the one-arena-per-stride rule (§7) makes the
   arena the single authority on a stream's byte pitch, and it is the value available at build
   time (the cache key holds arena refs, not streams).
2. Bind the element buffer: `bind_buffer(ELEMENT_ARRAY_BUFFER, get_arena(ebo_arena).
   buffer_id())`. This binding *is* VAO state, which is exactly why the VAO is specific to one
   EBO arena.

Per-mesh base offsets (each `SharedBuffer`'s `offset`, §7) are **not** baked — array draws pass
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
  feature flags it understands and the interned layout ref (§12) its attributes expect.
  `register_shader_family(ShaderFamilyId, sources, features, required_layout)` records one; the
  `ShaderFamilyId` is an interned family name. Builtin families come from the embedded
  `Shaders/*.glsl` sources (cppembed), so this stays within the existing shader pipeline.
- `register_shader_variant(ShaderVariantKey{ family, defines })` looks the key up in the variant
  cache (keyed by value equality on the key). **Hit:** return the cached `ShaderProgramRef`.
  **Miss:** run the family sources through the existing `ShaderPreprocessor`, which injects
  `#define <features[i]>` for every set bit *i* right after the `#version` line and rewrites
  the version itself to the platform's GLSL dialect (the sources target GLSL ES 3.00, which
  desktop GL rejects) — then compile and link, populate the `ShaderProgram` (`gl_handle`,
  `variant_key`, `required_layout`), insert it into the shader pool, record `key -> ref` in
  the cache, and return the ref.

The `defines` bitmask, not a string set, is what makes the key cheap to compare and hash; the
family's ordered feature list is the single place that maps a bit to its `#define` spelling.
`Material` selects appearance by holding the resolved `ShaderProgramRef`; the
`SnapshotBuilder` (or material-authoring code) is what turns "this material has a normal map
and is skinned" into a `ShaderVariantKey` and resolves it here.

**No second identity system.** Neither `ShaderProgram` nor `Material` carries a separate
numeric `id` for batching: the sort key packs the `ShaderProgramRef` / `MaterialRef` *index*,
which is unambiguous within one snapshot because snapshots are rebuilt from live refs every
frame (§5) — a slot reused across frames can never alias inside a single frame's draw list.
For skipping redundant binds, `GlStateCache` keys on the GL object names themselves (program
id, buffer id, texture id), which deferred deletion (§11) keeps unique among possibly-bound
objects.

## 15. Frustum culling

Culling runs over the flat renderable array, not over the hierarchy.

**Why not the hierarchy.** An earlier design kept an aggregate `Node::worldBounds` — a node's
own renderable bounds unioned with its children's — so that a subtree could be rejected
without visiting it. Two things go wrong with that. A scene graph's parenting is *logical*
(a hat on a head, a whole level under one node), and logical proximity has little to do with
spatial proximity, so a node whose children are spread out has a huge, mostly-empty union box
that almost never fails the frustum test. And every moving child dirties the bounds of every
ancestor up to the root. The hierarchy earns its keep for transform composition; it is a poor
spatial index.

**What `SnapshotBuilder` does instead** — three linear passes over the renderable store:

1. **`prepare_bounds`** transforms each `local_bounds` by its owner's world matrix into
   struct-of-arrays scratch: six `vector<float>` (`min_x`, `min_y`, … `max_z`) parallel to
   the store, plus per-item `drawable` and `cullable` flags. The scratch is kept across
   frames, so a steady-state build allocates nothing.
2. **`cull`** tests the six Gribb–Hartmann planes. Planes are the *outer* loop and items the
   inner one, because the positive-vertex test's choice of corner depends only on the sign of
   the plane's normal — hoisting it out picks a bounds array once per plane instead of once
   per item, and leaves an inner loop of six loads, three multiplies and a compare with no
   branches at all. That is the shape a compiler can vectorize; it is written as plain scalar
   code over separate arrays rather than with intrinsics, so it autovectorizes on both the
   desktop and the Emscripten target instead of depending on either.
3. **`extract_renderables`** builds `RenderItem`s for the survivors only.

Empty `local_bounds` still means "no bounds known": such an item is marked not-`cullable` and
is never rejected.

**Where this goes next.** The flat array is the right substrate for the two things that
follow. Below a few thousand objects, this linear SIMD-friendly sweep beats a tree — no
traversal, no pointer chasing, no branch mispredicts (Frostbite's "Culling the Battlefield",
GDC 2011, is the canonical measurement). Above that, a BVH or dynamic AABB tree built *over
the same array* is the standard answer, and it can be added without touching the per-item
test, which is already exactly what a BVH leaf would run.
