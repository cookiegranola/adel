#version 120

uniform sampler2D Render;

void CalculateExposure(inout vec3 color) {
	float exposureMax = 1.55f;
		  exposureMax *= mix(1.0f, 0.25f, timeSunriseSunset);
		  exposureMax *= mix(1.0f, 0.0f, timeMidnight);
		  exposureMax *= mix(1.0f, 0.25f, rainStrength);
	float exposureMin = 0.07f;
	float exposure = pow(eyeBrightnessSmooth.y / 240.0f, 6.0f) * exposureMax + exposureMin;

	//exposure = 1.0f;

	color.rgb /= vec3(exposure);
	color.rgb *= 350.0;
}

float   CalculateSunspot() {

	float curve = 1.0f;

	vec3 npos = normalize(GetWorldSpacePosition(texcoord.st).xyz);
	vec3 halfVector2 = normalize(-lightVector + npos);

	float sunProximity = 1.0f - dot(halfVector2, npos);

	return clamp(sunProximity - 0.9f, 0.0f, 0.1f) / 0.1f;

	//return sunSpot / (surface.glossiness * 50.0f + 1.0f);
	//return 0.0f;
}

void main(void)
{
    vec2 texcoord = vec2(gl_TexCoord[0]);
    vec4 finalColor = texture2D(Render, texcoord);

	vec3 color = finalColor.rgb;
	CalculateExposure(color);
		
	color *= 50.0 * BRIGHTNESS_LEVEL;
	const float p = TONEMAP_STRENGTH;
	color = (pow(color, vec3(p)) - color) / (pow(color, vec3(p)) - 1.0);
	color = pow(color, vec3(0.95 / 2.2));

	color *= 1.01;

	color = clamp(color, vec3(0.0), vec3(1.0));

	finalColor.rgb = color;
	gl_FragColor = finalColor;
}
