#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float sharpness = 1.0;

void main()
{
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    vec3 center = texture(screenTexture, TexCoords).rgb;
    vec3 sum = vec3(0.0);
    
    // Kernel wyostrzania
    sum += texture(screenTexture, TexCoords + texelSize * vec2(-1, -1)).rgb * -1.0;
    sum += texture(screenTexture, TexCoords + texelSize * vec2(-1,  1)).rgb * -1.0;
    sum += texture(screenTexture, TexCoords + texelSize * vec2( 1, -1)).rgb * -1.0;
    sum += texture(screenTexture, TexCoords + texelSize * vec2( 1,  1)).rgb * -1.0;
    sum += center * (5.0 + 4.0 * sharpness);
    
    FragColor = vec4(sum / (1.0 + 4.0 * sharpness), 1.0);
}