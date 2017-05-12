#version 120

uniform float BlendFactor;
uniform sampler2D Render;
uniform sampler2D Tex0;

void main()
{
    vec2 texCoord = vec2(gl_TexCoord[0]);
    gl_FragColor = texture2D(Render, texCoord)*BlendFactor + texture2D(Tex0, texCoord);
}
