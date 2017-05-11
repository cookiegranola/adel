#version 120

uniform sampler2D Render;

uniform float PixelSizeX;
uniform float PixelSizeY;

const float weights[5] = float[5](0.00390625, 0.03125, 0.109375, 0.21875, 0.2734375);


void main()
{
    vec4 blur = vec4(0.0);
    vec2 texcoord = vec2(gl_TexCoord[0]) + vec2(0.0, PixelSizeY/2.0);

	blur += texture2D(Render, texcoord - vec2(0.0, PixelSizeX * 8.0)) * weights[0];
	blur += texture2D(Render, texcoord - vec2(0.0, PixelSizeX * 6.0)) * weights[1];
	blur += texture2D(Render, texcoord - vec2(0.0, PixelSizeX * 4.0)) * weights[2];
	blur += texture2D(Render, texcoord - vec2(0.0, PixelSizeX * 2.0)) * weights[3];
	blur += texture2D(Render, vec2(gl_TexCoord[0])) * weights[4];
	blur += texture2D(Render, texcoord + vec2(0.0, PixelSizeX * 2.0)) * weights[3];
	blur += texture2D(Render, texcoord + vec2(0.0, PixelSizeX * 4.0)) * weights[2];
	blur += texture2D(Render, texcoord + vec2(0.0, PixelSizeX * 6.0)) * weights[1];
	blur += texture2D(Render, texcoord + vec2(0.0, PixelSizeX * 8.0)) * weights[0];

    gl_FragColor = blur;
}
