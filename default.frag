#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 viewPos;

#define MAX_LIGHTS 5
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);
    float shininess = 32.0;
    
    for (int i = 0; i < numLights; i++) {
        vec3 ambient = 0.2 * lightColors[i];
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColors[i];
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spec * lightColors[i];
        result += ambient + diffuse + specular;
    }
    result /= float(numLights);
    
    vec4 texColor = texture(texture1, TexCoord);
    FragColor = vec4(texColor.rgb * result, texColor.a);
}