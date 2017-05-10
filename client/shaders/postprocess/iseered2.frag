uniform sampler2D Render;

void main()
{
    vec4 color= texture2D(Render, gl_TexCoord[0].xy);
    color.r*= 1.0;
    color.g*= 0.0;
    color.b*= 0.0;

    gl_FragColor= color;
}

