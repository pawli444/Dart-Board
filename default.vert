#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 cameraMatrix;

void main()
{
    gl_Position = cameraMatrix * model * vec4(aPos, 1.0);
    TexCoords = vec2(1-aTexCoord.x, aTexCoord.y); 
}
