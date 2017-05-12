#version 120

uniform float BlendFactor;	
uniform float Exposure;

uniform sampler2D Render;
uniform sampler2D Tex0;

void main()
{
    vec2 texCoord = vec2(gl_TexCoord[0]);

    const float gamma = 2.2;
    vec3 bloomColor = texture2D(Tex0, texCoord).rgb;      
    vec3 sceneColor = texture2D(Render, texCoord).rgb;
    sceneColor += bloomColor * BlendFactor;
    // tone mapping
    vec3 color = vec3(1.0) - exp(-sceneColor * Exposure);
    //result = pow(result, vec3(1.0 / gamma));

    gl_FragColor = vec4(color, 1.0);    
}
