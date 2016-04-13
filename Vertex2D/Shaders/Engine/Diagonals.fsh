#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the obstacles
uniform float delta;

out vec4 out_color;

const vec4 q = vec4(1.0);

void main()
{
    vec4 p;
    p.x = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(2,0)).x;
    p.y = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(-2,0)).x;
    p.z = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(0,2)).x;
    p.w = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(0,-2)).x;

    float dx = 1.0;
    out_color = vec4(delta * dot(p,q) / (dx*dx),0.0, 0.0, 0.0);
}
