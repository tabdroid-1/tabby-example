$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2
 
#include <bgfx_shader.sh>

SAMPLER2D(u_Albedo0,  0);
SAMPLER2D(u_Albedo1,  1);
SAMPLER2D(u_Albedo2,  2);
SAMPLER2D(u_Albedo3,  3);
SAMPLER2D(u_Albedo4,  4);
SAMPLER2D(u_Albedo5,  5);
SAMPLER2D(u_Albedo6,  6);
SAMPLER2D(u_Albedo7,  7);
SAMPLER2D(u_Albedo8,  8);
SAMPLER2D(u_Albedo9,  9);
SAMPLER2D(u_Albedo10,  10);
SAMPLER2D(u_Albedo11,  11);
SAMPLER2D(u_Albedo12,  12);
SAMPLER2D(u_Albedo13,  13);
SAMPLER2D(u_Albedo14,  14);
SAMPLER2D(u_Albedo15,  15);

void main()
{    
    vec4 texColor = v_color0;

    if (int(v_texcoord1) == 0)
        texColor *= texture2D(u_Albedo0, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 1)
        texColor *= texture2D(u_Albedo1, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 2)
        texColor *= texture2D(u_Albedo2, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 3)
        texColor *= texture2D(u_Albedo3, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 4)
        texColor *= texture2D(u_Albedo4, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 5)
        texColor *= texture2D(u_Albedo5, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 6)
        texColor *= texture2D(u_Albedo6, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 7)
        texColor *= texture2D(u_Albedo7, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 8)
        texColor *= texture2D(u_Albedo8, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 9)
        texColor *= texture2D(u_Albedo9, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 10)
        texColor *= texture2D(u_Albedo10, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 11)
        texColor *= texture2D(u_Albedo11, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 12)
        texColor *= texture2D(u_Albedo12, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 13)
        texColor *= texture2D(u_Albedo13, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 14)
        texColor *= texture2D(u_Albedo14, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);
    if (int(v_texcoord1) == 15)
        texColor *= texture2D(u_Albedo15, vec2(v_texcoord0.x, v_texcoord0.y) * v_texcoord2);

    if (texColor.a == 0.0)
        discard;

	gl_FragColor = texColor;
}
