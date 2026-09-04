# Fading Blob (for lack of a better name)

I created this while finding out how to use framebuffers and textures. A small
rectangle orbits the centre of the window and leaves a trail behind it that
fades to black.

## How it works

The trail is accumulated in a pair of RGB textures that take turns being the
render target — the usual ping-pong arrangement:

1. SceneFader::draw_previous_scene binds the framebuffer with the current
   texture attached to it and draws the *other* texture over it as a
   full-screen quad, subtracting a small amount from every colour channel on
   the way. This is the previous frame, one step darker.
2. The scene itself — the rectangle — is drawn into the same framebuffer, on
   top of the darkened copy.
3. SceneFader::render_scene unbinds the framebuffer and draws the texture that
   was just composed to the screen, then swaps the two textures.

So every frame is the previous one darkened plus whatever was drawn this time,
and a pixel the rectangle has passed over decays towards black on its own.

## Neo and hand-written GL in one frame

Step 2 is the ordinary scene-graph pipeline, and steps 1 and 3 are hand-written
GL. The scene graph has no notion of a render target — there is no framebuffer
anywhere in Neo — and that is what lets the two compose: Renderer::render draws
into whatever framebuffer happens to be bound, so the fader binds one and the
renderer fills it, without either side knowing about the other.

Two things have to be got right at the seam, both in SceneFader:

  * Every bind the fader makes is invisible to Neo's GlStateCache, which elides
    a bind it believes is already current. The fader announces its own binds
    with notify_gl_state_changed(); without that the renderer skips binds it
    genuinely needs and draws the blob with the fader's program.

  * The renderer leaves the depth test enabled. A full-screen quad has no
    meaningful depth and the default framebuffer's depth buffer is never
    cleared here, so the fader disables the depth test before its own draws —
    otherwise the quad passes on the first frame, writes its depth, and fails
    on every frame after that.

The blob is drawn by a shader family of the example's own, registered at
FIRST_USER_SHADER_FAMILY. It is the only example that registers one — the
builtin Blinn-Phong and text families are registered inside the library — so it
doubles as the worked example of that extension point. The shader is unlit and
about as small as a family gets: a vec2 position at the fixed POSITION
location, the two matrices it needs out of the per-frame block, the model
matrix out of the per-draw block, and a colour out of the material block.

The orbit is a hub node that rotates and a child offset along +x. Composing the
two is resolve_transforms()'s job, so on_update sets one rotation per frame and
nothing touches the blob's own transform or its vertices.

## Why the fade is subtractive

The obvious way to fade towards black is to multiply each channel by something
just below 1 every frame, and that is what I tried first. It does not work on
an 8-bit texture: the result is rounded back to the nearest of 256 levels, so
once a channel is low enough that scaling it changes it by less than half a
level, it stops moving. Multiplying by 0.97 leaves everything below about 17
stuck where it is, and the trail never disappears — it just stops fading and
leaves a permanent smudge.

Subtracting a fixed step instead — 1/256, i.e. one level per frame — always
crosses a rounding boundary, so every pixel reaches black eventually. The cost
is that the fade is linear rather than exponential, and its speed depends on
the frame rate. Rendering the accumulation buffer at higher precision would
allow the multiplicative version, but that is more than this example needs.
