#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

in vec2 u_tex_position;
uniform sampler2D u_texture;
uniform vec3 u_color_delta;
out vec4 fragColor;

void main()
{
    vec4 tex_color = texture(u_texture, u_tex_position);
    vec3 sum = tex_color.rgb + u_color_delta;
    vec3 result = vec3(max(sum.r, 0.0),
    max(sum.g, 0.0),
    max(sum.b, 0.0));
    fragColor = vec4(result, 1.0);
}
