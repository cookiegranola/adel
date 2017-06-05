#version 120

//#define THRESHOLD 0.7f

uniform float Threshold;
uniform sampler2D Render;

void main()
{
    vec2 texcoord = vec2(gl_TexCoord[0]);
    vec4 color = texture2D(Render, texcoord);
	float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));	
    //gl_FragColor = step(Threshold, luma) * color;
	gl_FragColor = clamp((color - Threshold) / (1.0 - Threshold), 0.0, 1.0);
}
