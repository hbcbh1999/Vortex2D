#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordxp;
in vec2 v_texCoordxn;
in vec2 v_texCoordyp;
in vec2 v_texCoordyn;

uniform sampler2D u_texture;
uniform sampler2D u_pressure;

out vec4 colour_out;

void main()
{
    float x = texture(u_pressure, v_texCoord).x;
    float y = texture(u_texture, v_texCoord).y;

    colour_out = vec4(x, y, 0.0, 0.0);
}