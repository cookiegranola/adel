//#define THRESHOLD 0.7f

uniform float Threshold;
uniform sampler2D Render;

void main()
{
    vec2 texcoord = vec2(gl_TexCoord[0]);
    vec4 color = texture2D(Render, texcoord);
    gl_FragColor = step(Threshold, color) * color;
}
