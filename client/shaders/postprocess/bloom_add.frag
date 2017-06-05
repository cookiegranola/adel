#version 120

uniform float BlendFactor;
uniform sampler2D Render;
uniform sampler2D Tex0;

void main()
{
    vec2 texCoord = vec2(gl_TexCoord[0]);
    vec4 combine = texture2D(Render, texCoord)*BlendFactor;
	vec4 base = texture2D(Tex0, texCoord);
	base *= (1.0 - clamp(combine, 0.0, 1.0));
	gl_FragColor = base + combine;
}
