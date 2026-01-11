#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

in vec2 a_position;
in vec2 a_tex_position;
out vec2 u_tex_position;
void main()
{
    u_tex_position = a_tex_position;
    gl_Position = vec4(a_position, 0.0, 1.0);
}
