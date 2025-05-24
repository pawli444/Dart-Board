#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
    // W³aœciwoœci materia³u
    vec3 ambientColor = vec3(1.0);
    vec3 diffuseColor = vec3(1.0);
    vec3 specularColor = vec3(1.0);
    float shininess = 32.0;

    // Ambient
    vec3 ambient = 0.2 * ambientColor * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularColor * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    vec4 texColor = texture(texture1, TexCoord);
    FragColor = vec4(texColor.rgb * lighting, texColor.a);
}
