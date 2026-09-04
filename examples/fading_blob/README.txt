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
