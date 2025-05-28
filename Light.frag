#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    vec3 emissiveColor = lightColor * 5.0;
    FragColor = vec4(emissiveColor, 0.7); // Ustawienie alfa na 0.7 dla przezroczystoœci
}
