#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 viewPos;

#define MAX_LIGHTS 10 // Mo¿esz zwiêkszyæ maksymaln¹ liczbê œwiate³
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
        // Ambient
        vec3 ambient = 0.3 * lightColors[i]; // Zwiêkszona wartoœæ ambient
        
        // Diffuse
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColors[i];
        
        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spec * lightColors[i];
        
        // Atenuacja
        float distance = length(lightPositions[i] - FragPos);
        float constant = 1.0;
        float linear = 0.09;
        float quadratic = 0.032;
        float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
        
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        
        result += ambient + diffuse + specular;
    }
    
// Emisja (jeœli chcesz dodaæ efekt glow dla materia³ów emituj¹cych œwiat³o)
vec3 emission = vec3(0.0);

// Dodaj emisjê do wyniku koñcowego
vec4 texColor = texture(texture1, TexCoord);
FragColor = vec4(texColor.rgb * result + emission, texColor.a);
}
