#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_dirichlet;
uniform sampler2D u_neumann;
uniform float delta;

out vec4 out_color;

void main()
{
    vec4 p;
    p.x = textureOffset(u_dirichlet, v_texCoord, ivec2(1,0)).x;
    p.y = textureOffset(u_dirichlet, v_texCoord, ivec2(-1,0)).x;
    p.z = textureOffset(u_dirichlet, v_texCoord, ivec2(0,1)).x;
    p.w = textureOffset(u_dirichlet, v_texCoord, ivec2(0,-1)).x;

    vec4 q;
    q.x = textureOffset(u_neumann, v_texCoord, ivec2(2,0)).x;
    q.y = textureOffset(u_neumann, v_texCoord, ivec2(-2,0)).x;
    q.z = textureOffset(u_neumann, v_texCoord, ivec2(0,2)).x;
    q.w = textureOffset(u_neumann, v_texCoord, ivec2(0,-2)).x;

    float dx = 1.0;
    out_color = delta * (1.0 - max(p,q)) / (dx*dx);
}
