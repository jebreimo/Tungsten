# Scene graph — design notes

Companion to `docs/diagrams/src/scene_graph.mermaid`. The class diagram shows the shape of
the design; this document records the decisions and constraints that the diagram cannot
express.

## 1. Context

The design is a four-way split — scene / bridge / resource / render — with an immutable
`RenderSnapshot` as the only contract between the mutable scene graph and the renderer, and a
resource layer built on shared (sub-allocated) GL buffers. The portable path must run on
WebGL2 / GLSL ES 3.00 via Emscripten, which rules out several desktop-only features (§3).

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
rebuilt only after a structural edit, by appending the roots and then walking the output
vector itself as a queue — visiting breadth-first gives the invariant for free and keeps
siblings in insertion order.

Resist adding per-node dirty flags to skip unchanged subtrees: skipping work only pays when
the work costs more than deciding to skip it, and this pass is a sequential walk over two
arrays.

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

The set of kinds is therefore **closed**: a new kind means a new array, and there is no
dispatch on component type anywhere. User-defined components are the price; if they are ever
wanted, the retrofit is a type-erased registry keyed on `type_index`.

## 3. WebGL2 / GLES 3.00 constraints

The portable path must run on WebGL2 (= GLES 3.0). Features assumed by the original sketch
that are **not** available there, and what we do instead:

| Desktop feature | WebGL2 status | Portable approach |
| --- | --- | --- |
| `glDrawElementsBaseVertex` | Absent (GLES 3.2) | **Rebase indices to absolute offsets at upload.** No `baseVertex` field in the core `Mesh`; a base-vertex fast path is a desktop-only optimization, not part of the contract. |
| SSBO + `gl_DrawID` per-draw indexing | Absent (GLES 3.1+) | Per-draw data via a per-draw uniform, or an **instanced vertex attribute** (`vertexAttribDivisor`, which *is* in GLES 3.0). |
| Multi-draw indirect | Absent | One draw call per `RenderItem`; rely on sort-key batching to minimize state changes. |
| UBOs | **Available** | Used for per-frame and per-material data (see §4). |
| Sampler objects | **Available** (GLES 3.0) | Sampling state lives in interned sampler objects, not in texture parameters (§12.1). This does mean Neo requires an ES 3.0 / desktop context — never `ES_2` / `WEBGL_1`. |
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

**Binding 2 is one packed buffer, bound by range.** The renderer sorts both passes before
drawing, so it knows every item up front. It writes their blocks into one staging array,
spaced by `GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT` (every `glBindBufferRange` offset must be a
multiple of it), uploads that once, and then each draw only points binding 2 at its slice.
The padding is the price: a 112-byte block at a typical 256-byte alignment wastes 56%, or
256 KB per thousand items. The upload respecifies the whole store rather than sub-updating
it, which is the cheap way to avoid writing into memory the GPU may still be reading; a true
multi-frame ring with `glFenceSync` would go further and is available in GLES 3.0.

**Binding 1 is re-pointed at each material's own buffer.** `ResourceManager::create_material`
uploads the parameters once (`update_material_parameters` re-uploads them), so drawing a
material never re-uploads anything. This is the bind `GlStateCache` exists for: the sort key
groups items by material, so consecutive items sharing one bind nothing. A material either
has a buffer or it does not — a shader declaring `MaterialBlock` against one that has none is
rejected outright (`ShaderProgram::has_material_block`).

**Texture units follow the same fixed-convention idea.** A `ShaderFamily` lists its sampler
uniforms in unit order (`samplers`), and the `ShaderLibrary` points sampler *i* at texture
unit *i* right after linking each variant — samplers cannot live in UBOs, and their GLSL
default (unit 0 for every sampler) is never what a multi-textured material wants. The
renderer binds `Material::textures[i]` to unit *i*, so the two orders meet in the middle.
Units the program samples but the material leaves unfilled get the renderer's 1×1 white
dummy texture: a sampler must always see a complete texture (macOS's GL driver warns
otherwise), and white is the multiplicative identity if the shader reads it anyway.

**Every unit also gets a sampler object.** How a texture is filtered and wrapped is not
state baked into the GL texture object; it is a `SamplerDescriptor` interned in the
`SamplerRegistry` (§12.1) and named by `Texture::sampler`. The renderer binds that
sampler to the same unit as its texture, which overrides the texture object's own
parameters entirely. Binding one to *every* unit in `[0, sampler_count)` — the default
sampler for the units the material leaves unfilled — is deliberate: a unit left without
a sampler samples through whatever another subsystem last bound there, and Neo shares
its context with `TextRenderer` and the legacy examples.

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

**`SharedBuffer` is a non-owning slice, not shared ownership** — a plain, trivially
copyable `{ BufferArenaRef arena, uint32 offset, uint32 count }` naming a range of a
shared buffer. `offset` and `count` are in the arena's stride units (vertices or
indices), never bytes; byte offsets are computed via the arena's `stride()` only
where a GL call needs one.

**A `Mesh`'s vertex streams are plain `SharedBuffer`s.** Because the arena allocates
in vertex units, a slice's `offset` *is* the base vertex and its `count` *is* the
vertex count; the same holds for the `ebo` slice's first index and index count. A
per-stream stride field would only duplicate the arena's `stride()`, the single
authority on a stream's byte pitch (§13). So `Mesh` is just
`{ vao, streams, layout, ebo, index_type, primitive }`.

Resolving `arena` → `BufferArena` → `BufferHandle` goes through
`ResourceManager::get_arena`, at VAO build time and at allocate / upload / free time
— never per draw, since the buffer bindings are already VAO state. The arena ref is
generational (§6), so a stale slice fails validation rather than aliasing a regrown
buffer.

**The allocator is `Detail::BuddyAllocator`.** A `BufferArena` is `BufferHandle` +
`BuddyAllocator` (offset management) + growth logic. `allocate()` returns the unit
`offset`, or `nullopt` when the arena is full — it never grows on its own (see
below); `free()` hands that offset back; `grow()` reallocates the GL buffer and
replays `claim()` for every live allocation.

Two decisions this commits to:

- **Buddy's 2× internal fragmentation is accepted.** Rounding each request up to a
  power of two can waste half an allocation — fine for many small, similar
  allocations, worse for a few large oddly-sized meshes. If that shows up in
  profiling, a best-fit free-list fits behind the same `BufferArena` interface.
- **One arena per stride, allocating in vertex/index units.** This is what makes a
  slice's `offset` directly usable as a base vertex/index; buddy's power-of-two
  alignment then subsumes any explicit alignment field. One arena mixing strides
  would have to allocate in bytes and make stride and alignment do real work.

**Growth preserves offsets.** Doubling a buddy allocator's capacity leaves every
live block at its original offset (the "claim after doubling" test in
`tests/TungstenTest/test_BuddyAllocator.cpp` checks exactly this). So when an arena
grows, existing `SharedBuffer{offset,count}` values stay valid — only the GL
`BufferHandle` is rebound — and the immutable `RenderSnapshot` never needs its
meshes re-patched.

**The arena owns range; `ResourceManager` owns identity and growth policy.** An arena
does not know its own `BufferArenaRef`, so it never constructs a `SharedBuffer`:
`allocate(count)` returns a bare `optional<uint32>` offset, and
`ResourceManager::allocate(ref, count)` — the one caller that knows the ref — pairs
the two. When the arena is full that same method decides how much to grow, calls
`grow()`, and retries; being the single boundary, it is also where the displaced
buffer is retired and the affected VAOs rebuilt. The dependency stays
one-directional: `BufferArena` does not include `SharedBuffer`'s header.

Because cloning to grow changes the buffer id, the old buffer can still be in use
by in-flight draws (or, with a render thread, by the snapshot being rendered). It
is therefore moved onto a frame-tagged retirement queue and freed only once a
completed frame (single-threaded: the just-drawn frame; threaded: the latest
passed fence) proves nothing references it — the deferred-deletion half of the
generational scheme in §6.

**The VAO cache lives in `ResourceManager`, not in `Mesh`.** A VAO is shared by
every mesh with the same `(vbo arenas, ebo arena, layout)` triple, so no single
`Mesh` can own it: `get_vao` returns a non-owning id (`uint32`) and a `Mesh` stores
that. The cache is keyed on arena *refs* rather than live buffer ids, so growth
leaves entries valid; §13 covers the keying, the rebuild and eviction.

See `src/Tungsten/Neo/ResourceManager.{hpp,cpp}`.

## 8. Axis conventions and the 2D path

The scene's frame is right-handed: **-z forward, +y up, +x right**. Anything directional
reads that off its node's world matrix the same way — a camera looks along -z, and so does a
spot or directional light. It is one rule, not a per-component quirk.

**A camera is positioned by its node, not by itself.** `CameraComponent` holds only
projection parameters; the view matrix is the inverse of the node's world matrix. Parenting
that node to a vehicle's therefore makes the camera ride along — through any number of
intermediate boom or turret nodes — with no camera-specific code. To aim a node at a point
rather than parent it rigidly, `look_at_rotation(position, target, up)` returns the rotation
that puts -z on the target.

**Never parent a camera to a scaled node.** The view matrix inverts the whole world matrix,
so an ancestor's scale scales the rendered world. The camera's position still reads
correctly, which makes the symptom hard to place.

**Rebase orientations that arrive in vehicle axes.** Physics and flight models normally work
in +x forward, +y lateral (left), +z up, and a node driven straight from one faces sideways.
`x_forward_to_scene(rotation)` converts an orientation into the scene's frame;
`x_forward_to_scene_rotation()` is the mapping itself. Beware that in that frame a positive
turn about +y is nose *down* — the aviation convention measures pitch in an x forward / y
right / z down frame instead (`test_SceneGraph.cpp` pins the sign).

**2D and 3D share everything else** — one `Transform`, the resource and material layers, the
snapshot, and the renderer. The only branch is `CameraComponent::mode`: `PERSPECTIVE` (uses
`fov`) or `ORTHOGRAPHIC` (uses `ortho_size`). A 2D scene is nodes at `z = 0` rotated about z,
viewed orthographically; draw order goes through the render-layer / sort-key machinery rather
than a separate code path.

## 9. Integration with existing Tungsten

This is a forward-looking design; current code migrates toward it rather than being replaced
wholesale:

- All GL access continues through `IOglWrapper`, and resources use the existing RAII handle
  types, so the design stays within the repo's established conventions.

## 10. Resource manager decomposition

`ResourceManager` is a **thin facade** over one reusable container and four focused
collaborators it owns:

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
`GenerationalPool<T>`. A slot is `{ std::optional<T> value; uint32 generation; }` (empty
`value` marks a free slot), backed by a free-list of indices, with `insert` / `get` / `erase`
as the whole interface; `get` validates index, generation and occupancy, throwing otherwise.

The one non-obvious piece is that `erase` does **not** delete GL objects. It invokes an
`on_retire` callback with the departing value so the caller can move its GL handles onto the
`DeletionQueue` — the seam that keeps the pool ignorant of both GL and frames while still
driving deferred deletion.

The facade's `create_* / get_* / destroy_*` forward straight to `insert / get / erase`. Two
resource types keep a richer *create* path in front of `insert`: `BufferArena` because the
manager also constructs and grows it (`create_arena(usage, stride, capacity)`), and
`ShaderProgram` because it is only ever produced by `register_shader_variant` (§14), never
inserted directly by callers.

**The generation bump and deferred deletion are orthogonal.** The bump makes a stale ref fail
lookup instead of aliasing a reused slot (CPU-side); the `DeletionQueue` keeps the GL object
alive until no in-flight frame can read it (GPU-side). So a slot index may be reused on the
very next `insert` while the old GL object is still draining through the queue.

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

### 12.1 Sampler interning — the `SamplerRegistry`

Sampler descriptors are interned on exactly the same basis as layouts, and for the same
reasons: the set an application uses is small and bounded, and a sampler is never
individually freed, so the registry is a plain interning vector — no free-list, no
generations, no deletion path — and `SamplerRef` carries a fixed generation.

The one difference from `LayoutRegistry` is that a sampler *does* own a GL object, so
each entry keeps its `SamplerHandle` and the set is deleted when the registry dies.
Nothing is ever retired into the `DeletionQueue` (§11), because nothing is ever taken out
of service while frames are in flight. Creating and configuring a sampler object also
disturbs no binding state, so — unlike VAO baking (§13) — it must *not*
`notify_gl_state_changed()`.

A null `SamplerRef` resolves to a **default sampler**: linear, clamped, and explicitly
`mip_filter = NONE`, so a caller who does not care about filtering need say nothing and
a single-level texture stays complete. `SamplerDescriptor`'s own defaults ask for
mipmaps, which is why the default is spelled out rather than left implicit. It is
interned on first use, not in a constructor — `ResourceManager` issues no GL calls while
being constructed and must stay constructible before a context exists.

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
normal-mapping, alpha-clip, …).

- A **shader family** is a named GLSL source pair (vertex + fragment) plus the ordered list of
  feature flags it understands and the interned layout ref (§12) its attributes expect.
  `register_shader_family(ShaderFamilyId, sources, features, required_layout)` records one; the
  `ShaderFamilyId` is an interned family name. Builtin families come from the embedded
  `Shaders/*.glsl` sources (cppembed), so this stays within the existing shader pipeline.
- `register_shader_variant(ShaderVariantKey{ family, defines })` returns the cached ref on a
  hit. On a miss it runs the sources through `ShaderPreprocessor` — which injects
  `#define <features[i]>` for every set bit *i* and rewrites the `#version` to the platform's
  dialect, since the sources target GLSL ES 3.00 and desktop GL rejects it — then compiles,
  links, inserts into the shader pool and caches `key -> ref`.

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

**Do not reintroduce aggregate per-node bounds for a hierarchical subtree cull.** Parenting is
*logical* — a hat on a head, a whole level under one node — so a node whose children are
spread out has a huge, mostly-empty union box that almost never fails the frustum test, and
every moving child dirties its ancestors' bounds. The hierarchy earns its keep for transform
composition; it is a poor spatial index.

**`SnapshotBuilder` runs three linear passes over the renderable store:**

1. **`prepare_bounds`** transforms each `local_bounds` by its owner's world matrix into
   struct-of-arrays scratch: six `vector<float>` (`min_x`, `min_y`, … `max_z`) parallel to
   the store, plus per-item `drawable` and `cullable` flags. The scratch is kept across
   frames, so a steady-state build allocates nothing.
2. **`cull`** tests the six Gribb–Hartmann planes, with planes as the *outer* loop and items
   the inner one: the positive-vertex test's choice of corner depends only on the plane's
   sign, so hoisting it out leaves a branch-free inner loop. Written as plain scalar code over
   separate arrays rather than intrinsics, so it autovectorizes on both targets.
3. **`extract_renderables`** builds `RenderItem`s for the survivors only.

Empty `local_bounds` still means "no bounds known": such an item is marked not-`cullable` and
is never rejected.

**Where this goes next.** The flat array is the right substrate for the two things that
follow. Below a few thousand objects, this linear SIMD-friendly sweep beats a tree — no
traversal, no pointer chasing, no branch mispredicts (Frostbite's "Culling the Battlefield",
GDC 2011, is the canonical measurement). Above that, a BVH or dynamic AABB tree built *over
the same array* is the standard answer, and it can be added without touching the per-item
test, which is already exactly what a BVH leaf would run.

## 16. Text

Text is a *source* component compiled into an ordinary renderable, not a rendering path of
its own. `TextSystem::update(scene)` — called before `resolve_transforms()` — tessellates each
`TextComponent` into a `Mesh` and a `Material` and publishes them through a
`RenderableComponent` on the same node. `SnapshotBuilder` and `Renderer` are unchanged and
know nothing about glyphs; text gets culling, layers and sort-key batching for free.

**A text node therefore carries two components**: the `TextComponent` the application writes,
and the `RenderableComponent` the system owns and overwrites. Do not edit the latter by hand.

**There are no dirty flags and no setters.** `TextComponent` is a plain aggregate, and the
system finds its work by diffing the fields against `built`, its record of what it last
compiled. Nothing can change without the next `update()` noticing and nothing has to be
remembered to set; the cost is one string compare per item per frame. Styles are immutable
and shared (`shared_ptr<const TextStyle>`), so that diff is a *pointer* comparison across
every item using one style.

**The split that makes the frequent case cheap** is between what reaches the vertex data and
what reaches the material:

- font, line gap, alignment and the anchors are baked into the glyph positions — changing any
  of them re-tessellates that one item;
- colour lives in the `MaterialBlock`, so it never touches geometry.

**One material per (atlas, colour) pair**, interned on first use. A hundred cells sharing a
style therefore share one material and one bind, and `TextComponent::color_override` recolours
a single item by resolving a different `MaterialRef` rather than touching its vertices. The
materials are never retired, so an override must not be driven from a continuous value.

**Do not merge items into one mesh to cut draw calls.** One draw per item is the price of
per-item transforms and per-item culling; what is batched instead is state — one VAO for all
text, one material per colour.

**Reclamation is the system's job, not the scene's.** `Scene` drops a `TextComponent` when its
node dies but knows nothing about the GPU memory behind it, so `TextSystem` keeps a slot table
that each live component claims during the sweep; entries left unclaimed are released.

**Co-planar text relies on the transparent pass not writing depth** (§8's draw order). Text is
always transparent, and blended surfaces must leave the depth *mask* off — the test stays on,
so opaque geometry still occludes them — or the first one drawn punches a hole in everything
co-planar behind it, which silently defeats `render_layer`.
