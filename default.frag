#version 330 core

in vec2 TexCoord;
flat in int FaceType;

out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    if (FaceType == 0)
        FragColor = texture(texture1, TexCoord);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); 
}
