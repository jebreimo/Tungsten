# Resource management — design notes

A proposal for how meshes, materials, textures and shaders are organised and, more
importantly, how they are *unloaded*: who owns what, and how to tell what can be safely
removed. It builds on the existing `ResourceManager` rather than replacing it.

## 1. Context

The resource layer already solves half of the lifetime problem.

- **Generational refs.** Every resource is named by a `ResourceRef<T>` (`{index,
  generation}`). Destroying a resource bumps its slot's generation, so a stale ref fails
  validation on lookup instead of silently naming whatever reuses the slot.
- **Deferred deletion.** Everything that takes a GL object out of service retires it into
  the `DeletionQueue`, tagged with the current frame (`begin_frame`). `collect_garbage`
  deletes it only once the GPU is done with that frame.

So the question *"can the GPU still be using this?"* is answered. What is missing is the
other question: *"is anything on the CPU side still using this?"* — and its practical
form, *"what do I destroy to unload this model?"*

Two symptoms in the current code:

- `import_gltf` creates meshes, materials and textures but returns no record of them.
  Once it returns there is no way to unload what it loaded.
- `GltfWireframe` documents that destroying both a renderable's mesh and its wireframe
  mesh "frees those vertices twice". Ownership of the shared slices lives in a comment.

## 2. What actually depends on what

A mesh does not depend on a material; a `RenderableComponent` pairs them. The real graph
is:

```
RenderableComponent ──► Mesh ──► slices in BufferArenas, VertexLayout
                    └─► Material ──► Textures, Pipeline ──► ShaderVariant, VertexLayout
```

So "remove this mesh and everything it relies on" really means "remove everything that
*loading this model* brought in". That points to tracking ownership per **load unit**, not
per resource.

## 3. Refs stay weak

**`ResourceRef` gets no reference count.** Refs are copied into scene components and into
every frame's `RenderSnapshot`. Counting them would make those types non-trivial and put
counter traffic on the hot path, for a question that is only asked at load and unload
time. Refs remain what they are now: cheap, weak, revocable names.

Ownership lives one level up, in bundles (§4).

## 4. Resource bundles

A `ResourceBundle` records everything one load unit created, and destroying the bundle
destroys its contents.

```cpp
struct ResourceBundle
{
    std::vector<MeshRef> meshes;
    std::vector<MaterialRef> materials;
    std::vector<TextureRef> textures;
    std::vector<RenderTargetRef> render_targets;
    std::vector<SharedBuffer> slices;     // each freed once, independent of meshes
    std::vector<BufferArenaRef> arenas;   // if the load got arenas of its own
};

void ResourceManager::destroy_bundle(ResourceBundle& bundle);
```

- **Every loader returns one.** `GltfImport` gains a `ResourceBundle resources;` member,
  and so would any future importer or procedural generator.
- **Bundles match how content comes and goes:** per level, per model, per UI screen. They
  layer naturally — a long-lived "common" bundle plus short-lived per-level bundles.
- **The bundle owns slices, not the meshes.** Meshes become views onto slices the bundle
  owns, so each slice is freed exactly once however many meshes share it. This removes
  the wireframe double free. `destroy_bundle` erases the bundle's meshes without freeing
  their slices, so `destroy_mesh` itself is unchanged.
- **A slice in one of the bundle's own arenas need not be listed.** The arena goes with
  the bundle, and every slice in it with the arena. `import_gltf` relies on this: it
  lists its two arenas and no slices.

### 4.1 Destruction order

Nothing follows refs at destroy time, so the order mostly does not matter. The exception
is arenas: `destroy_arena` requires that no live mesh still draws from the arena, so they
go last.

1. materials
2. meshes
3. render targets
4. textures
5. slices
6. the bundle's own arenas

Everything still retires through the `DeletionQueue`, so frames in flight are unaffected.

## 5. Owned versus interned resources

The resources fall into two groups with different lifetime policies.

| Kind              | Examples                                                   | Policy                                                |
| ----------------- | ---------------------------------------------------------- | ----------------------------------------------------- |
| Owned, per load   | meshes, materials, textures, render targets, slices        | owned by a bundle, explicit lifetime                  |
| Interned / cached | vertex layouts, samplers, pipelines, shader variants, VAOs | deduplicated and shared by design, owned by no bundle |

The interned set grows with the number of *distinct combinations*, not with the amount of
content, so keeping it for the whole session is normally fine.

If it ever matters, reclaim it with **mark-and-sweep** at a natural boundary such as a
level transition:

1. Mark the pipeline of every live material.
2. Mark each marked pipeline's shader variant and vertex layout.
3. Mark the VAO of every live mesh, and the layout and samplers they use.
4. Sweep everything unmarked.

The graph is shallow and the pools are flat arrays, so this is cheap, and it needs no
counting anywhere.

## 6. Sharing across bundles

Two models may load the same `brick.png`. There are two options:

- **An asset cache in the import layer**, keyed by source path: `path → {TextureRef,
  count}`. Bundles acquire entries when they load and release them when destroyed. Counting
  is fine at this level because it is coarse: it changes once per load or unload, never per
  frame.
- **No counting at all:** load shared assets into the long-lived common bundle. Simpler,
  and enough when the shared set is known up front.

Either way the counting, if any, stays out of `ResourceRef` (§3).

## 7. Safe removal against the scene

A bundle's resources are referenced by scene components, so unloading a model is two
steps: remove its nodes, then destroy its bundle. To make that hard to get wrong:

- **Pair the subtree with the bundle.** The import already returns its `root` `NodeId`;
  give it an `unload(scene, resources)` that removes the subtree and then destroys the
  bundle, in that order.
- **Surface dangling refs in debug builds.** Generational refs already make a mistake
  detectable rather than undefined. `SnapshotBuilder` can log or assert when a component's
  ref fails to resolve, so a dangling component shows up immediately instead of being
  skipped silently.
- **Optional pre-destroy check.** A debug-only scan in `destroy_bundle` of the scene's flat
  `RenderableComponent` array for refs into the bundle about to be destroyed.

## 8. Next steps

1. ~~Add `ResourceBundle` and `ResourceManager::destroy_bundle`, with a mesh destroy
   path that leaves slices to the bundle.~~ Done.
2. ~~Return a bundle from `import_gltf`; make the wireframe meshes views onto the
   bundle's slices.~~ Done: `GltfImport::resources`. A failed import now also destroys
   what it had created.
3. ~~Add an unload path to `examples/gltf_viewer` as the usage reference.~~ Done:
   dropping a file on the window replaces the model.
4. Debug-build dangling-ref reporting in `SnapshotBuilder`.
5. Mark-and-sweep for interned resources and the path-keyed asset cache: only when a real
   need shows up.
