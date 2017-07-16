#version 120

const float exposure = 1.0;
const float decay = 1.0;
const float density = 1.0;
const float weight = 0.01;
#define NUM_SAMPLES 100

uniform float SunPositionX;
uniform float SunPositionY;
const vec2 lightPositionOnScreen = vec2(SunPositionX, SunPositionY);
uniform float Threshold;

uniform float PixelSizeX;
uniform float PixelSizeY;

uniform sampler2D Render;

void main()
{
	vec2 deltaTexCoord = vec2(gl_TexCoord[0].st - lightPositionOnScreen.xy);
	vec2 textCoo = gl_TexCoord[0].st;
	deltaTexCoord *= 1.0 /  float(NUM_SAMPLES) * density;
	float illuminationDecay = 1.0;

	for (int i = 0; i < NUM_SAMPLES; ++i) {
		textCoo -= deltaTexCoord;
                 vec4 sample = texture2D(firstPass, textCoo);

                 sample *= illuminationDecay * weight;

                 gl_FragColor += sample;

                 illuminationDecay *= decay;
	}

	gl_FragColor *= exposure;
}
