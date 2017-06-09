#version 120

const float exposure = 0.0034;
const float decay = 1.0;
const float density = 0.84;
const float weight = 5.65;
#define NUM_SAMPLES 100

const vec2 lightPositionOnScreen = vec2(0.5, 0.5);
uniform vec2 SunPosition;
uniform float Threshold;

uniform float PixelSizeX;
uniform float PixelSizeY;

uniform sampler2D Render;

void main()
{
    vec2 texcoord = vec2(gl_TexCoord[0]);
	vec2 deltaTexCoord = (texcoord - SunPosition);//lightPositionOnScreen);
	deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * density;
	float illuminationDecay = 1.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		texcoord -= deltaTexCoord;
		vec4 sample = texture2D(Render, texcoord);
		//sample.rgb = vec3((sample.r + sample.g + sample.b) == 0.0);
		sample *= illuminationDecay * weight;
		gl_FragColor += sample;
		illuminationDecay *= decay;
	}
    gl_FragColor *= exposure;
}
