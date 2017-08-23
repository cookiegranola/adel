uniform sampler2D ColorMapSampler;

vec2 offsetArray[11];

void main() 
{
	offsetArray[0] = vec2(0.0, 0.0);
	offsetArray[1] = vec2(-1.5 / float(SCREENX), 0.0);
	offsetArray[2] = vec2(1.5 / float(SCREENX), 0.0);
	offsetArray[3] = vec2(-2.5 / float(SCREENX), 0.0);
	offsetArray[4] = vec2(2.5 / float(SCREENX), 0.0);
	offsetArray[5] = vec2(-3.5 / float(SCREENX), 0.0);
	offsetArray[6] = vec2(3.5 / float(SCREENX), 0.0);
	offsetArray[7] = vec2(4.5 / float(SCREENX), 0.0);
	offsetArray[8] = vec2(-4.5 / float(SCREENX), 0.0);
	offsetArray[9] = vec2(5.5 / float(SCREENX), 0.0);
	offsetArray[10] = vec2(-5.5 / float(SCREENX), 0.0);

	vec4 BlurCol = vec4(0.0, 0.0, 0.0, 0.0);

	for(int i = 0;i < 11;++i)
		BlurCol += texture2D(ColorMapSampler,
			clamp(gl_TexCoord[0].xy + offsetArray[i] * 3.0,
				vec2(0.001, 0.01), vec2(0.999, 0.999)));
	
	BlurCol /= 11.0;
	
	gl_FragColor = BlurCol;
}