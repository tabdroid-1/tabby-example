#version 300 es
precision mediump float;

out vec4 o_Color;
out int o_EntityID;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;


flat in int v_EntityID;

uniform sampler2D u_FontAtlases[16];

float screenPxRange() {
	const float pxRange = 2.0; // set to distance field's pixel range
    vec2 unitRange;

	switch(int(v_TexIndex))
	{		
		case   0: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 0], 0)); break;
		case   1: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 1], 0)); break;
		case   2: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 2], 0)); break;
		case   3: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 3], 0)); break;
		case   4: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 4], 0)); break;
		case   5: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 5], 0)); break;
		case   6: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 6], 0)); break;
		case   7: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 7], 0)); break;
		case   8: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 8], 0)); break;
		case   9: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[ 9], 0)); break;
		case  10: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[10], 0)); break;
		case  11: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[11], 0)); break;
		case  12: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[12], 0)); break;
		case  13: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[13], 0)); break;
		case  14: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[14], 0)); break;
		case  15: unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[15], 0)); break;
	}

    vec2 screenTexSize = vec2(1.0)/fwidth(v_TexCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec4 texColor;
	vec3 msd;

	switch(int(v_TexIndex))
	{
		case  0: texColor = v_Color * texture(u_FontAtlases[ 0], v_TexCoord); msd = texture(u_FontAtlases[ 0], v_TexCoord).rgb; break;
		case  1: texColor = v_Color * texture(u_FontAtlases[ 1], v_TexCoord); msd = texture(u_FontAtlases[ 1], v_TexCoord).rgb; break;
		case  2: texColor = v_Color * texture(u_FontAtlases[ 2], v_TexCoord); msd = texture(u_FontAtlases[ 2], v_TexCoord).rgb; break;
		case  3: texColor = v_Color * texture(u_FontAtlases[ 3], v_TexCoord); msd = texture(u_FontAtlases[ 3], v_TexCoord).rgb; break;
		case  4: texColor = v_Color * texture(u_FontAtlases[ 4], v_TexCoord); msd = texture(u_FontAtlases[ 4], v_TexCoord).rgb; break;
		case  5: texColor = v_Color * texture(u_FontAtlases[ 5], v_TexCoord); msd = texture(u_FontAtlases[ 5], v_TexCoord).rgb; break;
		case  6: texColor = v_Color * texture(u_FontAtlases[ 6], v_TexCoord); msd = texture(u_FontAtlases[ 6], v_TexCoord).rgb; break;
		case  7: texColor = v_Color * texture(u_FontAtlases[ 7], v_TexCoord); msd = texture(u_FontAtlases[ 7], v_TexCoord).rgb; break;
		case  8: texColor = v_Color * texture(u_FontAtlases[ 8], v_TexCoord); msd = texture(u_FontAtlases[ 8], v_TexCoord).rgb; break;
		case  9: texColor = v_Color * texture(u_FontAtlases[ 9], v_TexCoord); msd = texture(u_FontAtlases[ 9], v_TexCoord).rgb; break;
		case 10: texColor = v_Color * texture(u_FontAtlases[10], v_TexCoord); msd = texture(u_FontAtlases[10], v_TexCoord).rgb; break;
		case 11: texColor = v_Color * texture(u_FontAtlases[11], v_TexCoord); msd = texture(u_FontAtlases[11], v_TexCoord).rgb; break;
		case 12: texColor = v_Color * texture(u_FontAtlases[12], v_TexCoord); msd = texture(u_FontAtlases[12], v_TexCoord).rgb; break;
		case 13: texColor = v_Color * texture(u_FontAtlases[13], v_TexCoord); msd = texture(u_FontAtlases[13], v_TexCoord).rgb; break;
		case 14: texColor = v_Color * texture(u_FontAtlases[14], v_TexCoord); msd = texture(u_FontAtlases[14], v_TexCoord).rgb; break;
		case 15: texColor = v_Color * texture(u_FontAtlases[15], v_TexCoord); msd = texture(u_FontAtlases[15], v_TexCoord).rgb; break;
	}


    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
    o_Color = mix(bgColor, v_Color, opacity);
	if (o_Color.a == 0.0)
		discard;
	
	o_EntityID = v_EntityID;
}
