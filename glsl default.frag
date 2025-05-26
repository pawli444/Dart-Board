#define MAX_LIGHTS 4
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

vec3 result = vec3(0.0);
for(int i = 0; i < numLights; ++i) {
    // klasyczny Phong/Blinn-Phong
    vec3 lightDir = normalize(lightPositions[i] - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColors[i];

    // prosty ambient
    vec3 ambient = 0.2 * lightColors[i];

    // specular (opcjonalnie)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * lightColors[i];

    result += ambient + diffuse + specular;
}
result = result / float(numLights); // opcjonalnie uœrednienie

FragColor = vec4(result * texture(texture1, TexCoords).rgb, 1.0);