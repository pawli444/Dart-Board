#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in int faceType;

out vec2 TexCoord;
flat out int FaceType;

uniform mat4 model;
uniform mat4 cameraMatrix;

void main()
{
    gl_Position = cameraMatrix * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    FaceType = faceType;
}
